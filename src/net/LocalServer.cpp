#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>

#include <event2/thread.h>

#include "Log.h"
#include "Options.h"
#include "Tileset.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "game/SimulationOptions.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "net/Server.h"
#include "net/SSLServer.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/ProtocolVersionPacket.h"
#include "packet/SelfTeleportedPacket.h"
#include "packet/TimePacket.h"
#include "realm/Overworld.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	LocalServer::LocalServer(std::shared_ptr<Server> server_, std::string_view secret_):
	server(std::move(server_)), secret(secret_) {
		server->addClient = [this](auto &, int new_client, std::string_view ip, int fd, bufferevent *event) {
			auto game_client = std::make_shared<RemoteClient>(*this, new_client, fd, ip, event);
			auto lock = server->lockClients();
			server->getClients().try_emplace(new_client, std::move(game_client));
			INFO("Adding " << new_client << " from " << ip);
		};

		server->closeHandler = [this](int client_id) {
			INFO("Closing " << client_id);
			std::shared_ptr<RemoteClient> client;
			{
				auto lock = server->lockClients();
				client = std::dynamic_pointer_cast<RemoteClient>(server->getClients().at(client_id));
			}
			assert(client);
			if (auto player = client->getPlayer())
				game->queueRemoval(player);
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
		if (onStop)
			onStop();
	}

	void LocalServer::send(GenericClient &client, std::string_view string) {
		server->send(client, string);
	}

	void LocalServer::saveUserData() {
		auto lock = game->players.sharedLock();

		game->database.writeUsers(game->players);

		for (const ServerPlayerPtr &player: game->players) {
			nlohmann::json json;
			{
				auto player_lock = player->sharedLock();
				player->toJSON(json);
			}
			game->database.writeUser(player->username, json);
		}
	}

	std::shared_ptr<ServerPlayer> LocalServer::loadPlayer(std::string_view username, std::string_view display_name) {
		if (!validateUsername(username))
			return nullptr;

		{
			nlohmann::json json;
			if (game->database.readUser(username, nullptr, &json))
				return ServerPlayer::fromJSON(*game, json);
		}

		RealmPtr overworld = game->getRealm(1);

		auto player = Entity::create<ServerPlayer>();
		player->username = username;
		player->displayName = display_name;
		player->token = generateToken(player->username);
		// overworld->add(player, overworld->randomLand);
		// overworld->add(player, Position(31, 82));
		// player->direction = Direction::Right;
		overworld->add(player, Position(-25, -36));
		player->init(*game);
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_pickaxe"));
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_shovel"));
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_axe"));
		player->inventory->add(ItemStack::withDurability(*game, "base:item/iron_hammer"));
		player->inventory->add(ItemStack(*game, "base:item/cave_entrance"));
		player->inventory->add(ItemStack(*game, "base:item/pump", 64));
		player->inventory->add(ItemStack(*game, "base:item/tank", 64));
		player->inventory->add(ItemStack(*game, "base:item/fluid_pipe", 64));
		player->inventory->add(ItemStack(*game, "base:item/centrifuge", 64));
		player->inventory->add(ItemStack(*game, "base:item/item_pipe", 64));
		player->inventory->add(ItemStack(*game, "base:item/geothermal_generator", 64));
		player->inventory->add(ItemStack(*game, "base:item/energy_pipe", 64));
		player->inventory->add(ItemStack(*game, "base:item/lava_flask", 64));
		player->inventory->add(ItemStack(*game, "base:item/chemical_reactor"));
		player->inventory->add(ItemStack(*game, "base:item/chemical", 64, {{"formula", "H2O"}}));
		player->inventory->add(ItemStack(*game, "base:item/chemical", 64, {{"formula", "H"}}));
		player->inventory->add(ItemStack(*game, "base:item/chemical", 64, {{"formula", "H"}}));
		player->inventory->add(ItemStack(*game, "base:item/chemical", 64, {{"formula", "O"}}));
		player->inventory->add(ItemStack(*game, "base:item/clay", 64));
		{
			auto lock = game->players.uniqueLock();
			game->players.insert(player);
		}
		return player;
	}

	Token LocalServer::generateToken(const std::string &username) const {
		return computeSHA3<Token>(secret + '/' + username);
	}

	void LocalServer::setupPlayer(RemoteClient &client) {
		auto player = client.getPlayer();
		auto realm = player->getRealm();
		INFO("Setting up player");
		player->weakClient = client.shared_from_this();
		player->notifyOfRealm(*player->getRealm());
		auto guard = client.bufferGuard();
		client.send(SelfTeleportedPacket(realm->id, player->getPosition()));
		client.send(TimePacket(game->time));
		auto lock = game->players.sharedLock();
		const EntityPacket packet(player);
		for (const auto &other_player: game->players)
			if (other_player != player)
				other_player->send(packet);
	}

	static std::shared_ptr<Server> global_server;
	static std::atomic_bool running = true;
	static std::condition_variable stopCV;
	static std::mutex stopMutex;

	bool LocalServer::validateUsername(std::string_view username) {
		if (username.empty() || 20 < username.size())
			return false;
		for (const char ch: username)
			if (!std::isalnum(ch) && ch != '_')
				return false;
		return true;
	}

	int LocalServer::main(int, char **) {
		evthread_use_pthreads();
		event_enable_debug_mode();

		std::string secret;

		if (std::filesystem::exists(".secret"))
			secret = readFile(".secret");
		else
			std::ofstream(".secret") << (secret = generateSecret(8));

#ifdef USE_SSL
		global_server = std::make_shared<SSLServer>(AF_INET6, "::0", 12255, "private.crt", "private.key", 2, 1024);
#else
		global_server = std::make_shared<Server>(AF_INET6, "::0", 12255, 2, 1024);
#endif

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGPIPE handler");

		auto stop_thread = std::thread([] {
			std::unique_lock lock(stopMutex);
			stopCV.wait(lock, [] { return !running.load(); });
			global_server->stop();
		});

		if (signal(SIGINT, +[](int) { running = false; stopCV.notify_all(); }) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGINT handler");

		auto game_server = std::make_shared<LocalServer>(global_server, secret);
		auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, game_server));

		game_server->onStop = [] {
			running = false;
			stopCV.notify_all();
		};

		constexpr const char *world_path = "world.db";

		const bool database_existed = std::filesystem::exists(world_path);
		game->openDatabase(world_path);
		game_server->game = game;
		game->initEntities();

		constexpr size_t seed = 1621;
		if (database_existed) {
			game->database.readAllRealms();
			INFO("Finished reading all realms from database.");
		} else {
			RealmPtr realm = Realm::create<Overworld>(*game, 1, Overworld::ID(), "base:tileset/monomap"_id, seed);
			realm->outdoors = true;
			std::default_random_engine rng;
			rng.seed(seed);
			WorldGen::generateOverworld(realm, seed, {}, {{-1, -1}, {1, 1}}, true);
			game->addRealm(realm->id, realm);
		}

		game->initInteractionSets();

		std::thread tick_thread = std::thread([&] {
			while (running) {
				game->tick();
				std::this_thread::sleep_for(std::chrono::milliseconds(TICK_PERIOD));
			}
		});

		game_server->run();
		tick_thread.join();
		stop_thread.join();

		return 0;
	}
}
