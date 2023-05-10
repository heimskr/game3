#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>

#include "net/ApplicationServer.h"
#include "net/Server.h"

namespace Game3 {
	class GameServer: public ApplicationServer {
		public:
			std::shared_ptr<Server> server;

			GameServer() = delete;
			GameServer(const GameServer &) = delete;
			GameServer(GameServer &&) = delete;
			GameServer(std::shared_ptr<Server>);

			~GameServer() override;

			GameServer & operator=(const GameServer &) = delete;
			GameServer & operator=(GameServer &&) = delete;

			void run() override;
			void stop() override;

			static int main(int argc, char **argv);
	};

}
