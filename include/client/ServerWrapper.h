#pragma once

#include "types/Types.h"
#include "util/FDWrapper.h"
#include "util/PipeWrapper.h"

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace Game3 {
	class DirectLocalClient;
	class DirectRemoteClient;
	class Server;
	class ServerGame;

	/** Encapsulates a server that runs inside the client to enable local singleplayer gameplay. */
	class ServerWrapper {
		public:
			std::function<void(std::string_view)> onLog;
			std::function<void(const std::exception &)> onError;

			ServerWrapper() = default;
			~ServerWrapper();

			void runInThread(size_t overworld_seed = -1);
			void run(size_t overworld_seed = -1);
			void stop();
			bool isRunning() const;
			bool waitUntilRunning(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
			inline uint16_t getPort() const { return port; }
			Token getOmnitoken() const;
			void save();
			std::shared_ptr<DirectRemoteClient> getDirectRemoteClient(const std::shared_ptr<DirectLocalClient> &);

		private:
			std::shared_ptr<Server> server;
			std::shared_ptr<ServerGame> game;
			std::shared_ptr<DirectRemoteClient> directRemoteClient;
			std::optional<PipeWrapper> logDataPipe;
			PipeWrapper logControlPipe;
			CloningFDWrapper logFDWrapper;

			uint16_t port{};

			std::atomic_bool running = false;
			std::atomic_bool started = false;
			std::atomic_bool threadActive = false;
			std::atomic_bool forceSave = false;
			std::mutex stopMutex;
			std::mutex startMutex;
			std::condition_variable stopCV;
			std::condition_variable saveCV;
			std::condition_variable startCV;
			std::thread runThread;
			std::thread logThread;

			static bool generateCertificate(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path);
	};
}
