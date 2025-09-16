#include "util/Log.h"
#include "data/GameDB.h"
#include "game/ServerGame.h"
#include "net/Server.h"
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
			auto ssl_server = Server::create("::1", 40000, "private.crt", "private.key", readFile(".secret"), 2, 1024);
			auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, std::make_pair(ssl_server, 1uz)));
			game->openDatabase(1 < args.size()? args[1] : "world.game3");
			ssl_server->game = game;
			INFO("Reading...");
			GameDB &database = game->getDatabase();
			database.readAllRealms();
			INFO("Writing...");
			database.writeAllRealms();
			SUCCESS("Done.");
			Timer::summary();
			Timer::clear();
			return 0;
		}

		std::cerr << "Unknown argument specified.\n";
		return 2;
	}
}
