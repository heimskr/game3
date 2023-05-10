#include <cctype>

#include <event2/thread.h>

#include "Log.h"
#include "net/GameClient.h"
#include "net/GameServer.h"
#include "net/Server.h"
#include "net/SSLServer.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	GameServer::GameServer(std::shared_ptr<Server> server_): server(std::move(server_)) {
		server->addClient = [this](auto &, int new_client, std::string_view ip) {
			auto game_client = std::make_unique<GameClient>(*this, new_client, ip);
			server->getClients().try_emplace(new_client, std::move(game_client));
			INFO("Adding " << new_client << " from " << ip);
		};

		server->closeHandler = [](int client_id) {
			INFO("Closing " << client_id);
		};

		server->messageHandler = [](GenericClient &client, std::string_view message) {
			INFO(client.id << " sent [" << message << ']');
		};
	}

	GameServer::~GameServer() {
		server->addClient = {};
		server->closeHandler = {};
		server->messageHandler = {};
	}

	void GameServer::run() {
		server->run();
	}

	void GameServer::stop() {
		server->stop();
	}

	static std::shared_ptr<Server> global_server;

	int GameServer::main(int argc, char **argv) {
		evthread_use_pthreads();
		global_server = std::make_shared<SSLServer>(AF_INET6, "::0", 12255, "private.crt", "private.key", 2);

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGPIPE handler");

		if (signal(SIGINT, +[](int) { global_server->stop(); }) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGINT handler");

		GameServer game_server(global_server);

		game_server.run();

		return 0;
	}
}
