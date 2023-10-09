#include "Log.h"
#include "net/NetError.h"
#include "net/Server.h"
#include "net/GenericClient.h"
#include "net/RemoteClient.h"

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <boost/bind/bind.hpp>

namespace Game3 {
	Server::Server(const std::string &ip_, uint16_t port_, const std::filesystem::path &certificate_path, const std::filesystem::path &key_path, size_t thread_count, size_t chunk_size):
	ip(ip_),
	port(port_),
	chunkSize(chunk_size),
	threadCount(thread_count),
	pool(thread_count),
	sslContext(asio::ssl::context::tls),
	context(thread_count),
	acceptor(context, asio::ip::tcp::endpoint(asio::ip::address::from_string(ip), port)) {
		if (thread_count < 1)
			throw std::invalid_argument("Cannot instantiate a Server with a thread count of zero");

		sslContext.use_certificate_chain_file(certificate_path);
		sslContext.use_private_key_file(key_path, asio::ssl::context::pem);
	}

	Server::~Server() {
		// while (!bufferEvents.empty())
		// 	remove(bufferEvents.begin()->second);
		stop();
	}

	/*
	ServerWorker::Worker(Server &server_, size_t buffer_size, size_t id_):
	server(server_), bufferSize(buffer_size), buffer(std::make_unique<char[]>(buffer_size)), base(makeBase()), id(id_) {
		if (base == nullptr)
			throw std::runtime_error("Couldn't allocate a new event_base");

		acceptEvent = event_new(base, -1, EV_PERSIST, &worker_acceptcb, this);
		if (acceptEvent == nullptr)
			throw std::runtime_error("Couldn't allocate acceptEvent");
		if (event_add(acceptEvent, nullptr) < 0) {
			char error[64] = "?";
			if (!strerror_r(errno, error, sizeof(error)) || strcmp(error, "?") == 0)
				throw std::runtime_error("Couldn't add acceptEvent (" + std::to_string(errno) + ')');
			throw std::runtime_error("Couldn't add acceptEvent: " + std::string(error));
		}
	}

	ServerWorker::~Worker() {
		if (acceptEvent) {
			event_free(acceptEvent);
			acceptEvent = nullptr;
		}

		if (base) {
			event_base_free(base);
			base = nullptr;
		}
	}

	void ServerWorker::work(size_t) {
		event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
	}

	void ServerWorker::stop() {
		if (base) {
			event_base_loopexit(base, nullptr);
			base = nullptr;
		}
	}

	event_base * ServerWorker::makeBase() {
		event_config *config = event_config_new();
		event_base *base = event_base_new_with_config(config);
		event_config_free(config);
		return base;
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
			try {
				buffer_event = bufferEvents.at(descriptor);
			} catch (const std::out_of_range &) {
				ERROR("Tried to find descriptor " << descriptor << " (number of bufferevents: " << bufferEvents.size() << ')');
				for (const auto &[desc, ev]: bufferEvents)
					ERROR("- " << desc);
				throw;
			}
		}
		return buffer_event;
	}

	//*/

	void Server::handleMessage(RemoteClient &client, std::string_view message) {
		if (onMessage)
			onMessage(client, message);
	}

	/*
	ssize_t Server::send(int client, std::string_view message, bool force) {
		if (!force) {
			std::shared_ptr<GenericClient> client_ptr;
			{
				std::unique_lock clients_lock(clientsMutex);
				client_ptr = allClients.at(client);
			}
			if (client_ptr->isBuffering()) {
				auto &buffer = client_ptr->sendBuffer;
				auto lock = buffer.uniqueLock();
				buffer.bytes.insert(buffer.bytes.end(), message.begin(), message.end());
				return static_cast<ssize_t>(message.size());
			}
		}

		try {
			return bufferevent_write(getBufferEvent(getDescriptor(client)), message.begin(), message.size());
		} catch (const std::out_of_range &err) {
			return -1;
		}
	}

	ssize_t Server::send(int client, const std::string &message, bool force) {
		return send(client, std::string_view(message), force);
	}
	//*/

	void Server::send(RemoteClient &client, std::string_view message, bool force) {
		if (!force && client.isBuffering()) {
			SendBuffer &buffer = client.sendBuffer;
			auto lock = buffer.uniqueLock();
			buffer.bytes.insert(buffer.bytes.end(), message.begin(), message.end());
			return;
		}

		asio::async_write(client.socket, asio::buffer(message), [this](const asio::error_code &errc, size_t length) {
			handleWrite(errc, length);
		});
	}

	void Server::send(RemoteClient &client, const std::string &message, bool force) {
		return send(client, std::string_view(message), force);
	}

	void Server::handleWrite(const asio::error_code &errc, size_t) {
		if (errc) {
			WARN("Write: " << errc.message());
		}
	}

	void Server::accept() {

	}

	/*
	void ServerWorker::removeClient(int client) {
		remove(server.getBufferEvent(server.getDescriptor(client)));
	}

	void ServerWorker::remove(bufferevent *buffer_event) {
		int descriptor = -1;
		{
			auto descriptors_lock = server.lockDescriptors();
			descriptor = server.bufferEventDescriptors.at(buffer_event);
			server.bufferEventDescriptors.erase(buffer_event);
			server.bufferEvents.erase(descriptor);
		}
		{
			auto clients_lock = server.lockClients();
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

	void ServerWorker::removeDescriptor(int descriptor) {
		remove(server.getBufferEvent(descriptor));
	}

	void ServerWorker::queueAccept(int new_fd) {
		{
			auto lock = lockAcceptQueue();
			acceptQueue.push_back(new_fd);
		}
		event_active(acceptEvent, 0, 0);
	}

	void ServerWorker::queueClose(int client_id) {
		queueClose(server.getBufferEvent(server.getDescriptor(client_id)));
	}

	void ServerWorker::queueClose(bufferevent *buffer_event) {
		if (evbuffer_get_length(bufferevent_get_output(buffer_event)) == 0) {
			remove(buffer_event);
		} else {
			auto lock = lockCloseQueue();
			closeQueue.insert(buffer_event);
		}
	}
	//*/

	void Server::run() {
		// if (!threads.empty())
		// 	throw std::runtime_error("Cannot run server: already running");

		connected = true;

		for (size_t i = 0; i < threadCount; ++i)
			asio::post(pool, boost::bind(&asio::io_context::run, &context));

		// acceptThread = std::thread([this] {
		// 	context.run
		// });

		// makeName();

		// for (size_t i = 0; i < threadCount; ++i)
		// 	workers.emplace_back(makeWorker(chunkSize, i));

		// for (size_t i = 0; i < threadCount; ++i)
		// 	threads.emplace_back(std::thread([this, i, &worker = *workers.at(i)] {
		// 		{
		// 			std::unique_lock lock(workersCVMutex);
		// 			workersCV.wait(lock, [this] { return workersReady.load(); });
		// 		}
		// 		worker.work(i);
		// 	}));

		// workersReady = true;
		// workersCV.notify_all();

		// acceptThread = std::thread([this] {
		// 	mainLoop();
		// });

		// for (auto &thread: threads)
		// 	thread.join();

		// acceptThread.join();
		// pool.join();
	}

	void Server::mainLoop() {
		// event_config *config = event_config_new();
		// base = event_base_new_with_config(config);
		// event_config_free(config);

		// if (base == nullptr) {
		// 	char error[64] = "?";
		// 	if (!strerror_r(errno, error, sizeof(error)) || strcmp(error, "?") == 0)
		// 		throw std::runtime_error("Couldn't initialize libevent (" + std::to_string(errno) + ')');
		// 	throw std::runtime_error("Couldn't initialize libevent: " + std::string(error));
		// }

		// evconnlistener *listener = evconnlistener_new_bind(base, listener_cb, this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_THREADSAFE, -1, name, nameSize);

		// if (listener == nullptr) {
		// 	event_base_free(base.exchange(nullptr));
		// 	char error[64] = "?";
		// 	if (!strerror_r(errno, error, sizeof(error)) || strcmp(error, "?") == 0)
		// 		throw std::runtime_error("Couldn't initialize libevent listener (" + std::to_string(errno) + ')');
		// 	throw std::runtime_error("Couldn't initialize libevent listener: " + std::string(error));
		// }

		// event_base_dispatch(base);
		// evconnlistener_free(listener);
		// event_base_free(base.exchange(nullptr));
	}

	/*
	void ServerWorker::accept(int new_fd) {
		int new_client = -1;

		{
			auto lock = server.lockClients();
			if (!server.freePool.empty()) {
				new_client = *server.freePool.begin();
				server.freePool.erase(new_client);
			} else
				new_client = ++server.lastClient;
			server.descriptors.emplace(new_client, new_fd);
			server.clients.erase(new_fd);
			server.clients.emplace(new_fd, new_client);
		}

		evutil_make_socket_nonblocking(new_fd);
		bufferevent *buffer_event = bufferevent_socket_new(base, new_fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);

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
			server.addClient(*this, new_client, ip, new_fd, buffer_event);
		}

		bufferevent_setcb(buffer_event, conn_readcb, conn_writecb, conn_eventcb, this);
		bufferevent_enable(buffer_event, EV_READ | EV_WRITE);
	}

	void ServerWorker::handleWriteEmpty(bufferevent *buffer_event) {
		bool contains = false;
		{
			auto lock = lockCloseQueue();
			if ((contains = closeQueue.contains(buffer_event)))
				closeQueue.erase(buffer_event);
		}

		if (contains)
			remove(buffer_event);
	}

	void ServerWorker::handleEOF(bufferevent *buffer_event) {
		remove(buffer_event);
	}

	void ServerWorker::handleRead(bufferevent *buffer_event) {
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
	//*/

	void Server::stop() {
		// for (auto &worker: workers)
		// 	worker->stop();
		// if (auto *loaded = base.load())
		// 	event_base_loopbreak(loaded);
		pool.join();
	}

	/*
	std::shared_ptr<ServerWorker> Server::makeWorker(size_t buffer_size, size_t id) {
		return std::make_shared<ServerWorker>(*this, buffer_size, id);
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
	//*/

	bool Server::close(RemoteClient &client) {
		client.socket.close();
		// bufferevent *buffer_event = client.event;
		// std::shared_ptr<Worker> worker;
		// {
		// 	auto lock = lockWorkerMap();
		// 	worker = workerMap.at(buffer_event);
		// }
		// worker->queueClose(buffer_event);
		return true;
	}

	/*
	void listener_cb(evconnlistener *, evutil_socket_t fd, sockaddr *, int, void *data) {
		auto *server = reinterpret_cast<Server *>(data);
		std::unique_lock lock(server->threadCursorMutex);
		server->workers.at(server->threadCursor)->queueAccept(fd);
		server->threadCursor = (server->threadCursor + 1) % server->threadCount;
	}

	void conn_readcb(bufferevent *buffer_event, void *data) {
		auto *worker = reinterpret_cast<ServerWorker *>(data);
		worker->handleRead(buffer_event);
	}

	void conn_writecb(bufferevent *buffer_event, void *data) {
		if (evbuffer_get_length(bufferevent_get_output(buffer_event)) == 0) {
			auto *worker = reinterpret_cast<ServerWorker *>(data);
			worker->handleWriteEmpty(buffer_event);
		}
	}

	void conn_eventcb(bufferevent *buffer_event, short events, void *data) {
		if ((events & BEV_EVENT_EOF) != 0)
			reinterpret_cast<ServerWorker *>(data)->handleEOF(buffer_event);
		else if ((events & (BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) != 0)
			reinterpret_cast<ServerWorker *>(data)->server.remove(buffer_event);
	}

	void signal_cb(evutil_socket_t, short, void *data) {
		auto *server = reinterpret_cast<Server *>(data);
		WARN("Received SIGINT.");
		server->stop();
	}

	void worker_acceptcb(evutil_socket_t, short, void *data) {
		auto *worker = reinterpret_cast<ServerWorker *>(data);
		auto lock = worker->lockAcceptQueue();
		while (!worker->acceptQueue.empty()) {
			const int descriptor = worker->acceptQueue.back();
			worker->acceptQueue.pop_back();
			worker->accept(descriptor);
		}
	}
	//*/
}
