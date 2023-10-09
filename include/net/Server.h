#pragma once

#include "threading/Lockable.h"

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

#include <asio.hpp>
#include <boost/bind/bind.hpp>

namespace Game3 {
	class RemoteClient;

	using RemoteClientPtr = std::shared_ptr<RemoteClient>;

	// class ServerWorker: public std::enable_shared_from_this<ServerWorker> {
	// 	public:
	// 		Server &server;
	// 		size_t bufferSize;
	// 		std::unique_ptr<char[]> buffer;
	// 		size_t id;

	// 		/** Maps descriptors to read buffers. */
	// 		std::map<int, std::string> readBuffers;
	// 		std::vector<int> acceptQueue;

	// 		explicit ServerWorker(Server &server_, size_t buffer_size, size_t id_);

	// 		virtual ~ServerWorker();

	// 		void removeClient(int client);
	// 		void work(size_t id);
	// 		virtual void accept(int new_fd);
	// 		void stop();
	// 		void queueAccept(int new_fd);
	// 		void queueClose(int client);
	// 		[[nodiscard]] auto lockReadBuffers() { return std::unique_lock(readMutex); }
	// 		[[nodiscard]] auto lockAcceptQueue() { return std::unique_lock(acceptQueueMutex); }

	// 		friend Server;

	// 	private:
	// 		std::recursive_mutex readMutex;
	// 		std::recursive_mutex acceptQueueMutex;
	// 		std::recursive_mutex closeQueueMutex;

	// 		Lockable<std::unordered_set<RemoteClientPtr>> closeQueue;

	// 		virtual void remove(RemoteClientPtr);
	// };

	class Server {
		protected:
			int af;
			std::string ip;
			int port;
			size_t chunkSize;
			int sock = -1;
			bool connected = false;
			std::atomic_bool closed {false};

			size_t threadCount = 0;
			asio::thread_pool pool;
			std::thread acceptThread;

			std::atomic_int lastID = 0;

			// std::vector<std::shared_ptr<ServerWorker>> workers;
			// std::atomic_bool workersReady = false;

			// std::condition_variable workersCV;
			// std::mutex workersCVMutex;

			void handleWrite(const asio::error_code &, size_t);

			bool removeClient(int);

		public:
			asio::io_context context;
			asio::ip::tcp::acceptor acceptor;
			std::string id = "server";

			// Lockable<std::unordered_map<RemoteClientPtr, std::shared_ptr<ServerWorker>>> workerMap;
			Lockable<std::unordered_set<RemoteClientPtr>> allClients;

			void makeName();

			std::function<void(RemoteClient &, std::string_view message)> onMessage;
			std::function<void(RemoteClient &)> onClose;
			std::function<void(RemoteClient &)> onAdd;

			Server(int af_, const std::string &ip_, uint16_t port_, size_t thread_count, size_t chunk_size = 1024);
			Server(const Server &) = delete;
			Server(Server &&) = delete;
			Server & operator=(const Server &) = delete;
			Server & operator=(Server &&) = delete;
			virtual ~Server();

			[[nodiscard]] inline int getPort() const { return port; }
			void handleMessage(RemoteClient &, std::string_view);
			void mainLoop();
			void send(RemoteClient &, std::string_view, bool force = false);
			void send(RemoteClient &, const std::string &, bool force = false);
			void run();
			void stop();
			// virtual std::shared_ptr<ServerWorker> makeWorker(size_t buffer_size, size_t id);
			// bool remove(bufferevent *);
			bool close(int client_id);
			bool close(RemoteClient &);

			[[nodiscard]]
			inline auto & getClients() { return allClients; }

			[[nodiscard]]
			inline const auto & getClients() const { return allClients; }
	};
}
