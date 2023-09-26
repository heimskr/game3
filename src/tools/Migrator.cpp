#include "Log.h"
#include "data/GameDB.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/SSLServer.h"
#include "tools/Migrator.h"
#include "util/FS.h"
#include "util/Timer.h"

namespace Game3 {
	int migrate(const std::vector<std::string> &args) {
		if (args.empty()) {
			std::cerr << "Please specify an argument.\n";
			return 1;
		}

		if (args.front() == "all") {
			auto ssl_server = std::make_shared<SSLServer>(AF_INET6, "::1", 40000, "private.crt", "private.key", 2, 1024);
			auto game_server = std::make_shared<LocalServer>(ssl_server, readFile(".secret"));
			auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, game_server));
			game->openDatabase(1 < args.size()? args[1] : "world.db");
			game_server->game = game;
			INFO("Reading...");
			game->database.readAllRealms();
			INFO("Writing...");
			game->database.writeAllRealms();
			SUCCESS("Done.");
			Timer::summary();
			return 0;
		}

		std::cerr << "Unknown argument specified.\n";
		return 2;
	}
}
