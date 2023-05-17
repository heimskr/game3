#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <poll.h>
#include <set>
#include <shared_mutex>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

#include "net/GenericClient.h"

namespace Game3 {
	void listener_cb(evconnlistener *, evutil_socket_t, sockaddr *, int socklen, void *);
	void conn_readcb(bufferevent *, void *);
	void conn_writecb(bufferevent *, void *);
	void conn_eventcb(bufferevent *, short, void *);
	void signal_cb(evutil_socket_t, short, void *);
	void worker_acceptcb(evutil_socket_t, short, void *);

	class Server {
		protected:
			class Worker: public std::enable_shared_from_this<Worker> {
				public:
					Server &server;
					size_t bufferSize;
					std::unique_ptr<char[]> buffer;
					event_base *base = nullptr;
					size_t id;

					/** Maps descriptors to read buffers. */
					std::map<int, std::string> readBuffers;

					std::unordered_set<bufferevent *> managedBufferEvents;

					std::vector<int> acceptQueue;

					explicit Worker(Server &server_, size_t buffer_size, size_t id_);

					virtual ~Worker();

					void removeClient(int client);
					void work(size_t id);
					virtual void accept(int new_fd);
					void handleWriteEmpty(bufferevent *);
					void handleEOF(bufferevent *);
					void stop();
					void queueAccept(int new_fd);
					void queueClose(int client);
					void queueClose(bufferevent *);
					[[nodiscard]] auto lockReadBuffers() { return std::unique_lock(readMutex); }
					[[nodiscard]] auto lockAcceptQueue() { return std::unique_lock(acceptQueueMutex); }

					friend Server;
					friend void conn_readcb(bufferevent *, void *);
					friend void conn_writecb(bufferevent *, void *);
					friend void worker_acceptcb(evutil_socket_t, short, void *);

				private:
					std::recursive_mutex readMutex;
					std::recursive_mutex acceptQueueMutex;
					std::recursive_mutex closeQueueMutex;

					event *acceptEvent = nullptr;

					std::unordered_set<bufferevent *> closeQueue;

					virtual void remove(bufferevent *);
					void removeDescriptor(int);
					void handleRead(bufferevent *);

					[[nodiscard]] auto lockCloseQueue() { return std::unique_lock(closeQueueMutex); }

					static event_base * makeBase();
			};

			struct SendBuffer {
				std::shared_mutex mutex;
				std::atomic_bool active = false;
				std::vector<char> bytes;
				SendBuffer() = default;
			};

			int af;
			std::string ip;
			int port;
			size_t chunkSize;
			int sock = -1;
			bool connected = false;
			std::atomic_bool closed {false};
			size_t threadCount;
			std::shared_mutex threadCursorMutex;
			size_t threadCursor = 0;

			std::vector<std::thread> threads;
			std::thread acceptThread;

			std::vector<std::shared_ptr<Worker>> workers;
			std::atomic_bool workersReady = false;
			std::condition_variable workersCV;
			std::mutex workersCVMutex;

			event_base *base = nullptr;
			event *signalEvent = nullptr;

			sockaddr *name = nullptr;
			size_t nameSize = 0;
			sockaddr_in  name4 {};
			sockaddr_in6 name6 {};

			bool removeClient(int);
			void flushBuffer(int client, SendBuffer &, bool force = false);

		public:
			std::string id = "server";

			/** Maps bufferevents to Workers. Lock workerMapMutex before using. */
			std::map<bufferevent *, std::shared_ptr<Worker>> workerMap;

			/** Lock clientsMutex before using. l*/
			int lastClient = -1;

			/** Maps descriptors to client IDs. Lock clientsMutex before using. */
			std::map<int, int> clients;

			/** Contains IDs that were previously used but are now available. Lock clientsMutex before using. */
			std::unordered_set<int> freePool;

			/** Maps client IDs to descriptors. Lock clientsMutex before using. */
			std::map<int, int> descriptors;

			/** Maps client IDs to client instances. Lock clientsMutex before using. */
			std::map<int, std::shared_ptr<GenericClient>> allClients;

			/** Maps client IDs to send buffers. Lock clientsMutex before using. */
			std::map<int, SendBuffer> sendBuffers;

			/** Maps descriptors to bufferevents. Lock descriptorsMutex before using. */
			std::map<int, bufferevent *> bufferEvents;

			/** Maps bufferevents to descriptors. Lock descriptorsMutex before using. */
			std::map<bufferevent *, int> bufferEventDescriptors;

			std::recursive_mutex workerMapMutex;
			std::recursive_mutex clientsMutex;
			std::recursive_mutex descriptorsMutex;

			void makeName();

			int getDescriptor(int client);
			bufferevent * getBufferEvent(int descriptor);

			[[nodiscard]] auto lockWorkerMap() { return std::unique_lock(workerMapMutex); }
			[[nodiscard]] auto lockDescriptors() { return std::unique_lock(descriptorsMutex); }

			std::function<void(GenericClient &, std::string_view message)> messageHandler;
			/** clientsMutex will be locked while this is called. */
			std::function<void(int client)> closeHandler;
			/** clientsMutex will be locked while this is called. */
			std::function<void(Worker &, int client, std::string_view ip)> addClient;

			Server(int af_, const std::string &ip_, uint16_t port_, size_t thread_count, size_t chunk_size = 1024);
			Server(const Server &) = delete;
			Server(Server &&) = delete;
			Server & operator=(const Server &) = delete;
			Server & operator=(Server &&) = delete;
			virtual ~Server();

			[[nodiscard]] inline int getPort() const { return port; }
			void handleMessage(GenericClient &, std::string_view);
			void mainLoop();
			ssize_t send(int client, std::string_view, bool force = false);
			ssize_t send(int client, const std::string &, bool force = false);
			void run();
			void stop();
			virtual std::shared_ptr<Worker> makeWorker(size_t buffer_size, size_t id);
			bool remove(bufferevent *);
			bool close(int client_id);
			bool close(GenericClient &);
			void startBuffering(int client);
			void flushBuffer(int client);
			void stopBuffering(int client);

			[[nodiscard]] inline decltype(allClients) & getClients() { return allClients; }
			[[nodiscard]] inline const decltype(allClients) & getClients() const { return allClients; }
			[[nodiscard]] auto lockClients() { return std::unique_lock(clientsMutex); }

			/** Given a buffer, this function returns {-1, *} if the message is still incomplete or {i, l} if the
			 *  buffer contains a complete message, where i is the index at which the message ends and l is the size of
			 *  the delimiter that ended the message. By default, a message is considered complete after the first
			 *  newline. */
			static std::pair<ssize_t, size_t> isMessageComplete(std::string_view);

			friend void listener_cb(evconnlistener *, evutil_socket_t, sockaddr *, int socklen, void *);
			friend void conn_readcb(bufferevent *, void *);
			friend void conn_writecb(bufferevent *, void *);
			friend void conn_eventcb(bufferevent *, short, void *);
			friend void signal_cb(evutil_socket_t, short, void *);
			friend void worker_acceptcb(evutil_socket_t, short, void *);
	};
}
