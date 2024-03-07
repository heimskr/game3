#pragma once

#include <filesystem>
#include <string>

namespace Game3 {
	class Server;

	/** Encapsulates a server that runs inside the client to enable local singleplayer gameplay. */
	class ServerWrapper {
		public:
			ServerWrapper() = default;

			void run();

		private:
			std::unique_ptr<Server> server;

			static bool generateCertificate(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path);
	};
}