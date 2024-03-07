#pragma once

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>

namespace Game3 {
	class Server;
	class ServerGame;

	/** Encapsulates a server that runs inside the client to enable local singleplayer gameplay. */
	class ServerWrapper {
		public:
			ServerWrapper() = default;

			void run(size_t overworld_seed = -1);
			void stop();

		private:
			std::shared_ptr<Server> server;
			std::shared_ptr<ServerGame> game;

			std::atomic_bool running = false;
			std::mutex stopMutex;
			std::condition_variable stopCV;
			std::condition_variable saveCV;

			static bool generateCertificate(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path);
	};
}