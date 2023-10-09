#pragma once

#include "Types.h"
#include "threading/Lockable.h"

#include <atomic>
#include <condition_variable>
#include <filesystem>
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
#include <asio/ssl.hpp>
#include <boost/bind/bind.hpp>

namespace Game3 {
	class RemoteClient;
	using RemoteClientPtr = std::shared_ptr<RemoteClient>;

	class ServerGame;
	class ServerPlayer;

	class Server {
		protected:
			std::string ip;
			int port;
			size_t chunkSize;
			int sock = -1;
			bool connected = false;
			std::atomic_bool closed {false};
			std::string secret;

			size_t threadCount = 0;
			std::atomic_int lastID = 0;

			void handleWrite(const asio::error_code &, size_t);
			bool removeClient(int);
			void accept();

		public:
			asio::ssl::context sslContext;
			asio::io_context context;
			asio::ip::tcp::acceptor acceptor;
			asio::executor_work_guard<asio::io_context::executor_type> workGuard;
			std::string id = "server";
			std::shared_ptr<ServerGame> game;
			std::function<void()> onStop;

			Lockable<std::unordered_set<RemoteClientPtr>> allClients;

			std::function<void(RemoteClient &, std::string_view message)> onMessage;
			std::function<void(RemoteClient &)> onClose;
			std::function<void(RemoteClient &)> onAdd;

			Server(const std::string &ip_, uint16_t port_, const std::filesystem::path &certificate_path, const std::filesystem::path &key_path, std::string_view secret_, size_t thread_count, size_t chunk_size = 1024);
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
			bool close(RemoteClient &);

			/** Writes every player's full data to the database. */
			void saveUserData();
			std::shared_ptr<ServerPlayer> loadPlayer(std::string_view username, std::string_view display_name);
			Token generateToken(const std::string &username) const;
			void setupPlayer(RemoteClient &);

			static bool validateUsername(std::string_view);
			static int main(int argc, char **argv);

			template <std::integral T>
			void send(RemoteClient &client, T value) {
				const T little = toLittle(value);
				send(client, std::string_view(reinterpret_cast<const char *>(&little), sizeof(T)));
			}

			[[nodiscard]]
			inline auto getChunkSize() const { return chunkSize; }

			[[nodiscard]]
			inline auto & getClients() { return allClients; }

			[[nodiscard]]
			inline const auto & getClients() const { return allClients; }
	};
}
