#include <cctype>

#include <event2/thread.h>

#include "Log.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "net/Server.h"
#include "net/SSLServer.h"
#include "packet/ProtocolVersionPacket.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	LocalServer::LocalServer(std::shared_ptr<Server> server_): server(std::move(server_)) {
		server->addClient = [this](auto &, int new_client, std::string_view ip) {
			auto game_client = std::make_shared<RemoteClient>(*this, new_client, ip);
			server->getClients().try_emplace(new_client, std::move(game_client));
			INFO("Adding " << new_client << " from " << ip);
		};

		server->closeHandler = [](int client_id) {
			INFO("Closing " << client_id);
		};

		server->messageHandler = [](GenericClient &generic_client, std::string_view message) {
			INFO(generic_client.id << " sent [" << message << ']');
			RemoteClient &client = dynamic_cast<RemoteClient &>(generic_client);
			ProtocolVersionPacket packet;
			client.send(packet);
		};
	}

	LocalServer::~LocalServer() {
		server->addClient = {};
		server->closeHandler = {};
		server->messageHandler = {};
	}

	void LocalServer::run() {
		server->run();
	}

	void LocalServer::stop() {
		server->stop();
	}

	void LocalServer::send(const GenericClient &client, std::string_view string) {
		send(client.id, string);
	}

	void LocalServer::send(int id, std::string_view string) {
		server->send(id, string);
	}

	static std::shared_ptr<Server> global_server;
	static bool running = true;

	int LocalServer::main(int, char **) {
		evthread_use_pthreads();
		global_server = std::make_shared<SSLServer>(AF_INET6, "::0", 12255, "private.crt", "private.key", 2);

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGPIPE handler");

		if (signal(SIGINT, +[](int) { running = false; global_server->stop(); }) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGINT handler");

		auto game_server = std::make_shared<LocalServer>(global_server);
		auto game = std::make_shared<ServerGame>(game_server);

		std::thread tick_thread = std::thread([&] {
			while (running) {
				game->tick();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		});

		game_server->run();
		tick_thread.join();

		return 0;
	}
}
