#include "Log.h"
#include "Options.h"
#include "graphics/Tileset.h"
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
#include "realm/ShadowRealm.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/ShadowRealm.h"
#include "worldgen/WorldGen.h"

#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>

namespace Game3 {
	LocalServer::LocalServer(std::shared_ptr<Server> server_, std::string_view secret_):
	server(std::move(server_)), secret(secret_) {
		server->onAdd = [](RemoteClient &client) {
			INFO("Adding connection from " << client.ip);
		};

		server->onClose = [this](RemoteClient &client) {
			INFO("Closing connection from " << client.ip);
			if (auto player = client.getPlayer())
				game->queueRemoval(player);
		};

		server->onMessage = [](RemoteClient &client, std::string_view message) {
			client.handleInput(message);
		};
	}

	LocalServer::~LocalServer() {
		server->onAdd = {};
		server->onClose = {};
		server->onMessage = {};
	}

	void LocalServer::run() {
		server->run();
	}

	void LocalServer::stop() {
		server->stop();
		if (onStop)
			onStop();
	}

	void LocalServer::send(RemoteClient &client, std::string_view string) {
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
			game->database.writeUser(player->username, json, std::nullopt);
		}
	}

	std::shared_ptr<ServerPlayer> LocalServer::loadPlayer(std::string_view username, std::string_view display_name) {
		if (!validateUsername(username))
			return nullptr;

		{
			nlohmann::json json;
			if (game->database.readUser(std::string(username), nullptr, &json, nullptr))
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
		const InventoryPtr inventory = player->getInventory();
		{
			auto inventory_lock = inventory->uniqueLock();
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_pickaxe"));
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_shovel"));
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_axe"));
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_hammer"));
			inventory->add(ItemStack(*game, "base:item/cave_entrance"));
			inventory->add(ItemStack(*game, "base:item/pump", 64));
			inventory->add(ItemStack(*game, "base:item/tank", 64));
			inventory->add(ItemStack(*game, "base:item/centrifuge", 64));
			inventory->add(ItemStack(*game, "base:item/geothermal_generator", 64));
			inventory->add(ItemStack(*game, "base:item/chemical_reactor"));
			inventory->add(ItemStack(*game, "base:item/energy_pipe", 64));
			inventory->add(ItemStack(*game, "base:item/fluid_pipe", 64));
			inventory->add(ItemStack(*game, "base:item/item_pipe", 64));
			inventory->add(ItemStack(*game, "base:item/lava_flask", 64));
			inventory->add(ItemStack(*game, "base:item/chemical", 64, {{"formula", "H2O"}}));
			inventory->add(ItemStack(*game, "base:item/chemical", 64, {{"formula", "H"}}));
			inventory->add(ItemStack(*game, "base:item/chemical", 64, {{"formula", "O"}}));
		}
		game->addPlayer(player);
		return player;
	}

	Token LocalServer::generateToken(const std::string &username) const {
		return computeSHA3_512<Token>(secret + '/' + username);
	}

	void LocalServer::setupPlayer(RemoteClient &client) {
		auto player = client.getPlayer();
		auto realm = player->getRealm();
		INFO("Setting up player");
		player->weakClient = client.shared_from_this();
		player->notifyOfRealm(*realm);
		auto guard = client.bufferGuard();
		client.send(SelfTeleportedPacket(realm->id, player->getPosition()));
		client.send(TimePacket(game->time));
		auto lock = game->players.sharedLock();
		const EntityPacket packet(player);
		for (const auto &other_player: game->players) {
			if (other_player != player) {
				other_player->notifyOfRealm(*realm);
				other_player->send(packet);
			}
		}
	}

	namespace {
		std::shared_ptr<Server> global_server;
		std::atomic_bool running = true;
		std::condition_variable stopCV;
		std::mutex stopMutex;
		std::condition_variable saveCV;
	}

	bool LocalServer::validateUsername(std::string_view username) {
		if (username.empty() || 20 < username.size())
			return false;
		for (const char ch: username)
			if (!std::isalnum(ch) && ch != '_')
				return false;
		return true;
	}

	int LocalServer::main(int argc, char **argv) {
		// evthread_use_pthreads();
		// event_enable_debug_mode();

		std::string secret;

		if (std::filesystem::exists(".secret"))
			secret = readFile(".secret");
		else
			std::ofstream(".secret") << (secret = generateSecret(8));

		uint16_t port = 12255;
		if (3 <= argc) {
			try {
				port = parseNumber<uint16_t>(argv[2]);
			} catch (const std::invalid_argument &) {}
		}

#ifdef USE_SSL
		global_server = std::make_shared<SSLServer>(AF_INET6, "::0", port, "private.crt", "private.key", 2, 1024);
#else
		global_server = std::make_shared<Server>(AF_INET6, "::0", port, 2, 1024);
#endif

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGPIPE handler");

		auto stop_thread = std::thread([] {
			std::unique_lock lock(stopMutex);
			stopCV.wait(lock, [] { return !running.load(); });
			global_server->stop();
		});

		if (signal(SIGINT, +[](int) { running = false; stopCV.notify_all(); saveCV.notify_all(); }) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGINT handler");

		auto game_server = std::make_shared<LocalServer>(global_server, secret);
		auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, game_server));

		game_server->onStop = [] {
			running = false;
			stopCV.notify_all();
			saveCV.notify_all();
		};

		const char *world_path = "world.db";
		if (4 <= argc)
			world_path = argv[3];

		const bool database_existed = std::filesystem::exists(world_path);
		game->openDatabase(world_path);
		game_server->game = game;

		size_t seed = 1621;
		if (std::filesystem::exists(".seed")) {
			seed = parseNumber<size_t>(strip(readFile(".seed")));
			INFO("Using custom seed \e[1m" << seed << "\e[22m");
		}

		if (database_existed) {
			game->database.readAllRealms();
			Timer::summary();
			Timer::clear();
			INFO("Finished reading all realms from database.");
		} else {
			RealmPtr realm = Realm::create<Overworld>(*game, 1, Overworld::ID(), "base:tileset/monomap", seed);
			realm->outdoors = true;
			std::default_random_engine rng;
			rng.seed(seed);
			WorldGen::generateOverworld(realm, seed, {}, {{-1, -1}, {1, 1}}, true);
			game->addRealm(realm->id, realm);
		}

		if (!game->hasRealm(-1)) {
			RealmPtr shadow = Realm::create<ShadowRealm>(*game, -1, ShadowRealm::ID(), "base:tileset/monomap", seed);
			shadow->outdoors = false;
			std::default_random_engine rng;
			rng.seed(seed);
			WorldGen::generateShadowRealm(shadow, seed, {}, {{-1, -1}, {1, 1}}, true);
			game->addRealm(shadow->id, shadow);
		}

		game->initEntities();
		game->initInteractionSets();

		std::thread tick_thread([&] {
			while (running) {
				if (!game->tickingPaused)
					game->tick();
				std::this_thread::sleep_for(std::chrono::milliseconds(TICK_PERIOD));
			}
		});

		std::mutex save_mutex;
		std::chrono::seconds save_period{120};

		std::thread save_thread([&] {
			std::chrono::time_point last_save = std::chrono::system_clock::now();

			while (running) {
				std::unique_lock lock{save_mutex};
				saveCV.wait_for(lock, save_period, [&] {
					return !running || save_period <= std::chrono::system_clock::now() - last_save;
				});

				if (save_period <= std::chrono::system_clock::now() - last_save) {
					INFO("Autosaving...");
					game->tickingPaused = true;
					game->database.writeAllRealms();
					auto player_lock = game->players.sharedLock();
					game->database.writeUsers(game->players);
					game->tickingPaused = false;
					INFO("Autosaved.");
					last_save = std::chrono::system_clock::now();
				}
			}
		});

		game_server->run();
		tick_thread.join();
		stop_thread.join();
		save_thread.join();

		return 0;
	}
}
