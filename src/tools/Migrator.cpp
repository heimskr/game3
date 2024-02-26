#include "Log.h"
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
			auto ssl_server = std::make_shared<Server>("::1", 40000, "private.crt", "private.key", readFile(".secret"), 2, 1024);
			auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, std::make_pair(ssl_server, size_t(1))));
			game->openDatabase(1 < args.size()? args[1] : "world.db");
			ssl_server->game = game;
			INFO_("Reading...");
			GameDB &database = game->getDatabase();
			database.readAllRealms();
			INFO_("Writing...");
			database.writeAllRealms();
			SUCCESS_("Done.");
			Timer::summary();
			Timer::clear();
			return 0;
		}

		std::cerr << "Unknown argument specified.\n";
		return 2;
	}
}
