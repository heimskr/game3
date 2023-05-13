#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>

#include <event2/thread.h>

#include "Log.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "net/Server.h"
#include "net/SSLServer.h"
#include "packet/ProtocolVersionPacket.h"
#include "realm/Overworld.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	LocalServer::LocalServer(std::shared_ptr<Server> server_, std::string_view secret_):
	server(std::move(server_)), secret(secret_) {
		server->addClient = [this](auto &, int new_client, std::string_view ip) {
			auto game_client = std::make_shared<RemoteClient>(*this, new_client, ip);
			server->getClients().try_emplace(new_client, std::move(game_client));
			INFO("Adding " << new_client << " from " << ip);
		};

		server->closeHandler = [](int client_id) {
			INFO("Closing " << client_id);
		};

		server->messageHandler = [](GenericClient &generic_client, std::string_view message) {
			generic_client.handleInput(message);
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

	void LocalServer::readUsers(const std::filesystem::path &path) {
		userDatabase = nlohmann::json::parse(readFile(path));
		displayNames.clear();
		for (const auto &[username, user_info]: userDatabase)
			displayNames.insert(user_info.displayName);
	}

	void LocalServer::saveUsers(const std::filesystem::path &path) {
		std::ofstream(path) << nlohmann::json(userDatabase).dump();
	}

	std::optional<std::string> LocalServer::authenticate(const std::string &username, Token token) const {
		if (validateUsername(username))
			if (auto iter = userDatabase.find(username); iter != userDatabase.end() && iter->second.token == token)
				return iter->second.displayName;
		return std::nullopt;
	}

	PlayerPtr LocalServer::loadPlayer(std::string_view username, std::string_view display_name) {
		if (!validateUsername(username))
			return nullptr;

		const std::filesystem::path path = "world/users/" + std::string(username);

		if (std::filesystem::exists(path))
			return Player::fromJSON(*game, nlohmann::json::parse(readFile(path)));

		auto overworld = game->realms.at(1);

		auto player = Entity::create<Player>();
		player->username = username;
		player->displayName = display_name;
		player->token = generateToken(player->username);
		overworld->add(player);
		player->position = overworld->randomLand;
		player->init(*game);
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_pickaxe"));
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_shovel"));
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_axe"));
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_hammer"));
		player->inventory->add(ItemStack(*game, "base:item/cave_entrance"));
		game->players.insert(player);
		userDatabase.try_emplace(player->username, player->username, player->displayName, player->token);
		return player;
	}

	bool LocalServer::hasUsername(const std::string &username) const {
		return userDatabase.contains(username);
	}

	bool LocalServer::hasDisplayName(const std::string &display_name) const {
		return displayNames.contains(display_name);
	}

	Token LocalServer::generateToken(const std::string &username) const {
		return computeSHA3<Token>(secret + '/' + username);
	}

	static std::shared_ptr<Server> global_server;
	static std::atomic_bool running = true;

	bool LocalServer::validateUsername(std::string_view username) {
		if (username.empty())
			return false;
		for (const char ch: username)
			if (!std::isalnum(ch) && ch != '_')
				return false;
		return true;
	}

	int LocalServer::main(int, char **) {
		evthread_use_pthreads();

		std::string secret;

		if (std::filesystem::exists(".secret")) {
			secret = readFile(".secret");
		} else {
			secret = generateSecret(8);
			std::ofstream ofs(".secret");
			ofs << secret;
		}

		if (std::filesystem::exists("world")) {
			if (!std::filesystem::is_directory("world"))
				throw std::runtime_error("Path \"world\" exists but isn't a directory");
		} else
			std::filesystem::create_directory("world");

		if (std::filesystem::exists("world/users")) {
			if (!std::filesystem::is_directory("world/users"))
				throw std::runtime_error("Path \"world/users\" exists but isn't a directory");
		} else
			std::filesystem::create_directory("world/users");

		global_server = std::make_shared<SSLServer>(AF_INET6, "::0", 12255, "private.crt", "private.key", 2, 1024);
		// global_server = std::make_shared<Server>(AF_INET6, "::0", 12255, 2, 1024);

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGPIPE handler");

		if (signal(SIGINT, +[](int) { running = false; global_server->stop(); }) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGINT handler");

		auto game_server = std::make_shared<LocalServer>(global_server, secret);

		if (std::filesystem::exists("users.json"))
			game_server->readUsers("users.json");

		auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, game_server));
		game->initEntities();

		constexpr size_t seed = 1621;
		auto realm = Realm::create<Overworld>(*game, 1, Overworld::ID(), "base:tileset/monomap"_id, seed);
		realm->outdoors = true;
		std::default_random_engine rng;
		rng.seed(seed);
		WorldGen::generateOverworld(realm, seed, {}, {{-1, -1}, {1, 1}}, true);
		game->realms.emplace(realm->id, realm);
		game->activeRealm = realm;
		game->initInteractionSets();
		game_server->game = game;

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
