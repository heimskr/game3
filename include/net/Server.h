#pragma once

#include "types/Types.h"
#include "threading/Lockable.h"

#include "lib/ASIO.h"

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

namespace Game3 {
	class GenericClient;
	using GenericClientPtr = std::shared_ptr<GenericClient>;

	class ServerGame;
	class ServerPlayer;

	class Server: public std::enable_shared_from_this<Server> {
		private:
			std::string ip;
			int port;
			size_t chunkSize;
			bool connected = false;
			std::atomic_bool closed {false};
			std::string secret;

			size_t threadCount = 0;
			std::atomic_int lastID = 0;

			Lockable<std::unordered_set<GenericClientPtr>> allClients;

			void accept();

			Server(const std::string &ip_, uint16_t port_, const std::filesystem::path &certificate_path, const std::filesystem::path &key_path, std::string_view secret_, size_t thread_count, size_t chunk_size = 8192);

		public:
			asio::ssl::context sslContext;
			asio::io_context context;
			asio::ip::tcp::acceptor acceptor;
			asio::executor_work_guard<asio::io_context::executor_type> workGuard;
			std::string id = "server";
			std::shared_ptr<ServerGame> game;
			std::function<void()> onStop;

			std::function<void(GenericClient &, std::string_view message)> onMessage;
			std::function<void(GenericClient &)> onClose;
			std::function<void(GenericClient &)> onAdd;

			Server(const Server &) = delete;
			Server(Server &&) = delete;
			Server & operator=(const Server &) = delete;
			Server & operator=(Server &&) = delete;
			virtual ~Server();

			[[nodiscard]] inline int getPort() const { return port; }
			void handleMessage(GenericClient &, std::string_view);
			void mainLoop();
			void run();
			void stop();
			bool close(GenericClientPtr);

			/** Writes every player's full data to the database. */
			void saveUserData();
			std::shared_ptr<ServerPlayer> loadPlayer(std::string_view username, std::string_view display_name);
			Token generateToken(const std::string &username) const;
			void setupPlayer(GenericClient &);

			static bool validateUsername(std::string_view);
			static int main(int argc, char **argv);

			template <std::integral T>
			void send(GenericClient &client, T value) {
				const T little = toLittle(value);
				send(client, std::string(reinterpret_cast<const char *>(&little), sizeof(T)), true);
			}

			template <typename... Args>
			static std::shared_ptr<Server> create(Args &&...args) {
				Server *server = new Server(std::forward<Args>(args)...);
				try {
					return std::shared_ptr<Server>(server);
				} catch (...) {
					delete server;
					throw;
				}
			}

			[[nodiscard]]
			inline auto getChunkSize() const { return chunkSize; }

			[[nodiscard]]
			inline auto & getClients() { return allClients; }

			[[nodiscard]]
			inline const auto & getClients() const { return allClients; }

		private:
			void send(GenericClient &, std::string, bool force);

		friend class ServerWrapper;
	};

	using ServerPtr = std::shared_ptr<Server>;
}
