#include <iostream>

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Log.h"
#include "net/NetError.h"
#include "net/Server.h"

namespace Game3 {
	Server::Server(int af_, const std::string &ip_, uint16_t port_, size_t thread_count, size_t chunk_size):
	af(af_), ip(ip_), port(port_), chunkSize(chunk_size), threadCount(thread_count) {
		if (thread_count < 1)
			throw std::invalid_argument("Cannot instantiate a Server with a thread count of zero");
	}

	Server::~Server() {
		while (!bufferEvents.empty())
			remove(bufferEvents.begin()->second);
		stop();
	}

	Server::Worker::Worker(Server &server_, size_t buffer_size, size_t id_):
	server(server_), bufferSize(buffer_size), buffer(std::make_unique<char[]>(buffer_size)), base(event_base_new()), id(id_) {
		if (base == nullptr)
			throw std::runtime_error("Couldn't allocate a new event_base");

		acceptEvent = event_new(base, -1, EV_PERSIST, &worker_acceptcb, this);
		if (acceptEvent == nullptr)
			throw std::runtime_error("Couldn't allocate acceptEvent");
		if (event_add(acceptEvent, nullptr) < 0) {
			char error[64] = "?";
			if (!strerror_r(errno, error, sizeof(error)))
				throw std::runtime_error("Couldn't add acceptEvent (" + std::to_string(errno) + ')');
			throw std::runtime_error("Couldn't add acceptEvent: " + std::string(error));
		}
	}

	Server::Worker::~Worker() {
		event_free(acceptEvent);
		event_base_free(base);
	}

	void Server::Worker::work(size_t) {
		event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
	}

	void Server::Worker::stop() {
		event_base_loopexit(base, nullptr);
	}

	void Server::makeName() {
		name4 = {.sin_family  = AF_INET,  .sin_port  = htons(port), .sin_addr = {}, .sin_zero = {0}};
		name6 = {.sin6_family = AF_INET6, .sin6_port = htons(port), .sin6_flowinfo = {}, .sin6_addr = {}, .sin6_scope_id = 0};

		int status;

		if (af == AF_INET) {
			name = reinterpret_cast<sockaddr *>(&name4);
			nameSize = sizeof(name4);
			status = inet_pton(AF_INET, ip.c_str(), &name4.sin_addr.s_addr);
		} else if (af == AF_INET6) {
			name = reinterpret_cast<sockaddr *>(&name6);
			nameSize = sizeof(name6);
			status = inet_pton(AF_INET6, ip.c_str(), &name6.sin6_addr);
		} else
			throw std::invalid_argument("Unsupported or invalid address family: " + std::to_string(af));

		if (status != 1)
			throw std::invalid_argument("Couldn't parse IP address \"" + ip + "\"");
	}

	int Server::getDescriptor(int client) {
		int descriptor;
		{
			auto lock = lockClients();
			descriptor = descriptors.at(client);
		}
		return descriptor;
	}

	bufferevent * Server::getBufferEvent(int descriptor) {
		bufferevent *buffer_event;
		{
			auto lock = lockDescriptors();
			buffer_event = bufferEvents.at(descriptor);
		}
		return buffer_event;
	}

	void Server::handleMessage(GenericClient &client, std::string_view message) {
		if (messageHandler)
			messageHandler(client, message);
	}

	ssize_t Server::send(int client, std::string_view message) {
		try {
			return bufferevent_write(getBufferEvent(getDescriptor(client)), message.begin(), message.size());
		} catch (const std::out_of_range &err) {
			return -1;
		}
	}

	ssize_t Server::send(int client, const std::string &message) {
		return send(client, std::string_view(message));
	}

	void Server::Worker::removeClient(int client) {
		remove(server.getBufferEvent(server.getDescriptor(client)));
	}

	void Server::Worker::remove(bufferevent *buffer_event) {
		int descriptor = -1;
		{
			auto descriptors_lock = server.lockDescriptors();
			descriptor = server.bufferEventDescriptors.at(buffer_event);
			server.bufferEventDescriptors.erase(buffer_event);
			server.bufferEvents.erase(descriptor);
		}
		{
			auto client_lock = server.lockClients();
			const int client_id = server.clients.at(descriptor);
			if (server.closeHandler)
				server.closeHandler(client_id);
			server.allClients.erase(client_id);
			server.freePool.insert(client_id);
			server.descriptors.erase(client_id);
			server.clients.erase(descriptor);
		}
		{
			auto read_lock = lockReadBuffers();
			readBuffers.erase(descriptor);
		}
		{
			auto worker_lock = server.lockWorkerMap();
			server.workerMap.erase(buffer_event);
		}
		bufferevent_free(buffer_event);
	}

	void Server::Worker::removeDescriptor(int descriptor) {
		remove(server.getBufferEvent(descriptor));
	}

	void Server::Worker::queueAccept(int new_fd) {
		{
			auto lock = lockAcceptQueue();
			acceptQueue.push_back(new_fd);
		}
		event_active(acceptEvent, 0, 0);
	}

	void Server::Worker::queueClose(int client_id) {
		queueClose(server.getBufferEvent(server.getDescriptor(client_id)));
	}

	void Server::Worker::queueClose(bufferevent *buffer_event) {
		if (evbuffer_get_length(bufferevent_get_output(buffer_event)) == 0) {
			remove(buffer_event);
		} else {
			auto lock = lockCloseQueue();
			closeQueue.insert(buffer_event);
		}
	}

	void Server::run() {
		if (!threads.empty())
			throw std::runtime_error("Cannot run server: already running");

		connected = true;

		makeName();

		acceptThread = std::thread([this] {
			mainLoop();
		});

		for (size_t i = 0; i < threadCount; ++i) {
			workers.emplace_back(makeWorker(chunkSize, i));
			threads.emplace_back(std::thread([i, &worker = *workers.back()] {
				worker.work(i);
			}));
		}

		for (auto &thread: threads)
			thread.join();

		acceptThread.join();
		threads.clear();
		workers.clear();
	}

	void Server::mainLoop() {
		base = event_base_new();
		if (base == nullptr) {
			char error[64] = "?";
			if (!strerror_r(errno, error, sizeof(error)))
				throw std::runtime_error("Couldn't initialize libevent (" + std::to_string(errno) + ')');
			throw std::runtime_error("Couldn't initialize libevent: " + std::string(error));
		}

		evconnlistener *listener = evconnlistener_new_bind(base, listener_cb, this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_THREADSAFE, -1, name, nameSize);

		if (listener == nullptr) {
			event_base_free(base);
			char error[64] = "?";
			if (!strerror_r(errno, error, sizeof(error)))
				throw std::runtime_error("Couldn't initialize libevent listener (" + std::to_string(errno) + ')');
			throw std::runtime_error("Couldn't initialize libevent listener: " + std::string(error));
		}

		event_base_dispatch(base);
		evconnlistener_free(listener);
		event_base_free(base);
	}

	void Server::Worker::accept(int new_fd) {
		int new_client = -1;

		{
			auto lock = server.lockClients();
			if (!server.freePool.empty()) {
				new_client = *server.freePool.begin();
				server.freePool.erase(new_client);
			} else
				new_client = ++server.lastClient;
			server.descriptors.emplace(new_client, new_fd);
			server.clients[new_fd] = new_client;
		}

		evutil_make_socket_nonblocking(new_fd);
		bufferevent *buffer_event = bufferevent_socket_new(base, new_fd, BEV_OPT_CLOSE_ON_FREE);

		if (buffer_event == nullptr) {
			event_base_loopbreak(base);
			throw std::runtime_error("buffer_event is null");
		}

		{
			auto lock = server.lockDescriptors();
			if (server.bufferEvents.contains(new_fd))
				throw std::runtime_error("File descriptor " + std::to_string(new_fd) + " already has a bufferevent struct");
			server.bufferEvents.emplace(new_fd, buffer_event);
			server.bufferEventDescriptors.emplace(buffer_event, new_fd);
		}

		{
			auto lock = server.lockWorkerMap();
			server.workerMap.emplace(buffer_event, shared_from_this());
		}

		bufferevent_setcb(buffer_event, conn_readcb, conn_writecb, conn_eventcb, this);
		bufferevent_enable(buffer_event, EV_READ | EV_WRITE);

		if (server.addClient) {
			std::string ip;
			sockaddr_in6 addr6 {};
			socklen_t addr6_len = sizeof(addr6);
			if (getpeername(new_fd, reinterpret_cast<sockaddr *>(&addr6), &addr6_len) == 0) {
				char ip_buffer[INET6_ADDRSTRLEN];
				if (inet_ntop(addr6.sin6_family, &addr6.sin6_addr, ip_buffer, sizeof(ip_buffer)) != nullptr)
					ip = ip_buffer;
				else
					WARN("inet_ntop failed: " << strerror(errno));
			} else
				WARN("getpeername failed: " << strerror(errno));
			if (std::string_view(ip).substr(0, 7) == "::ffff:" && ip.find('.') != std::string::npos)
				ip.erase(0, 7);
			auto lock = server.lockClients();
			server.addClient(*this, new_client, ip);
		}
	}

	void Server::Worker::handleWriteEmpty(bufferevent *buffer_event) {
		bool contains = false;
		{
			auto lock = lockCloseQueue();
			if ((contains = closeQueue.contains(buffer_event)))
				closeQueue.erase(buffer_event);
		}

		if (contains)
			remove(buffer_event);
	}

	void Server::Worker::handleEOF(bufferevent *buffer_event) {
		remove(buffer_event);
	}

	void Server::Worker::handleRead(bufferevent *buffer_event) {
		const int descriptor = bufferevent_getfd(buffer_event);
		if (descriptor == -1)
			throw std::runtime_error("descriptor is -1 in Worker::handleRead");

		evbuffer *input = bufferevent_get_input(buffer_event);

		size_t readable = evbuffer_get_length(input);

		try {
			auto lock = lockReadBuffers();
			std::string &str = readBuffers[descriptor];
			str.reserve(bufferSize);

			auto &clients = server.clients;
			auto clients_lock = server.lockClients();
			const int client_id = clients.at(descriptor);

			std::shared_ptr<GenericClient> client;
			try {
				client = server.allClients.at(client_id);
			} catch (const std::out_of_range &) {
				// Seems the client isn't ready for reading yet.
				ERROR("Client " << client_id << " doesn't have an instance yet");
				return;
			}

			while (0 < readable) {
				size_t to_read = std::min(bufferSize, readable);
				const bool use_max_read = 0 < client->maxRead;

				if (use_max_read)
					to_read = std::min(to_read, client->maxRead);

				const int byte_count = evbuffer_remove(input, buffer.get(), to_read);

				if (use_max_read)
					client->maxRead -= byte_count;

				if (byte_count < 0)
					throw NetError("Reading", errno);

				server.handleMessage(*client, {buffer.get(), static_cast<size_t>(byte_count)});
				readable = evbuffer_get_length(input);
			}
		} catch (const std::runtime_error &err) {
			ERROR(err.what());
			remove(buffer_event);
		}
	}

	void Server::stop() {
		event_base_loopbreak(base);
		for (auto &worker: workers)
			worker->stop();
	}

	std::shared_ptr<Server::Worker> Server::makeWorker(size_t buffer_size, size_t id) {
		return std::make_shared<Server::Worker>(*this, buffer_size, id);
	}

	bool Server::remove(bufferevent *buffer_event) {
		std::shared_ptr<Worker> worker;
		{
			auto lock = lockWorkerMap();
			if (!workerMap.contains(buffer_event))
				return false;
			worker = workerMap.at(buffer_event);
		}
		worker->remove(buffer_event);
		return true;
	}

	bool Server::removeClient(int client) {
		int descriptor = -1;
		{
			auto lock = lockClients();
			descriptor = descriptors.at(client);
		}

		bufferevent *buffer_event = nullptr;
		{
			auto lock = lockDescriptors();
			buffer_event = bufferEvents.at(descriptor);
		}

		return remove(buffer_event);
	}

	bool Server::close(int client_id) {
		bufferevent *buffer_event = nullptr;
		try {
			buffer_event = getBufferEvent(getDescriptor(client_id));
		} catch (const std::out_of_range &) {
			return false;
		}
		std::shared_ptr<Worker> worker;
		{
			auto lock = lockWorkerMap();
			worker = workerMap.at(buffer_event);
		}
		worker->queueClose(buffer_event);
		return true;
	}

	bool Server::close(GenericClient &client) {
		return close(client.id);
	}

	std::pair<ssize_t, size_t> Server::isMessageComplete(std::string_view view) {
		const size_t found = view.find('\n');
		return found == std::string::npos? std::pair<ssize_t, size_t>(-1, 0) : std::pair<ssize_t, size_t>(found, 1);
	}

	void listener_cb(evconnlistener *, evutil_socket_t fd, sockaddr *, int, void *data) {
		auto *server = reinterpret_cast<Server *>(data);
		server->workers.at(server->threadCursor)->queueAccept(fd);
		server->threadCursor = (server->threadCursor + 1) % server->threadCount;
	}

	void conn_readcb(bufferevent *buffer_event, void *data) {
		auto *worker = reinterpret_cast<Server::Worker *>(data);
		worker->handleRead(buffer_event);
	}

	void conn_writecb(bufferevent *buffer_event, void *data) {
		if (evbuffer_get_length(bufferevent_get_output(buffer_event)) == 0) {
			auto *worker = reinterpret_cast<Server::Worker *>(data);
			worker->handleWriteEmpty(buffer_event);
		}
	}

	void conn_eventcb(bufferevent *buffer_event, short events, void *data) {
		if ((events & BEV_EVENT_EOF) != 0)
			reinterpret_cast<Server::Worker *>(data)->handleEOF(buffer_event);
		else if ((events & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) != 0)
			reinterpret_cast<Server::Worker *>(data)->server.remove(buffer_event);
	}

	void signal_cb(evutil_socket_t, short, void *data) {
		auto *server = reinterpret_cast<Server *>(data);
		WARN("Received SIGINT.");
		server->stop();
	}

	void worker_acceptcb(evutil_socket_t, short, void *data) {
		auto *worker = reinterpret_cast<Server::Worker *>(data);
		auto lock = worker->lockAcceptQueue();
		while (!worker->acceptQueue.empty()) {
			const int descriptor = worker->acceptQueue.back();
			worker->acceptQueue.pop_back();
			worker->accept(descriptor);
		}
	}
}
