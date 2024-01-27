#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "game/SimulationOptions.h"
#include "net/NetError.h"
#include "net/Server.h"
#include "net/GenericClient.h"
#include "net/RemoteClient.h"
#include "packet/EntityMoneyChangedPacket.h"
#include "packet/RecipeListPacket.h"
#include "packet/SelfTeleportedPacket.h"
#include "packet/TimePacket.h"
#include "realm/Overworld.h"
#include "realm/ShadowRealm.h"
#include "recipe/CraftingRecipe.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/ShadowRealm.h"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fstream>

namespace Game3 {
	Server::Server(const std::string &ip_, uint16_t port_, const std::filesystem::path &certificate_path, const std::filesystem::path &key_path, std::string_view secret_, size_t thread_count, size_t chunk_size):
	ip(ip_),
	port(port_),
	chunkSize(chunk_size),
	secret(secret_),
	threadCount(thread_count),
	sslContext(asio::ssl::context::tls),
	context(thread_count),
	acceptor(context, asio::ip::tcp::endpoint(asio::ip::address::from_string(ip), port)),
	workGuard(asio::make_work_guard(context)) {
		if (thread_count < 1)
			throw std::invalid_argument("Cannot instantiate a Server with a thread count of zero");

		sslContext.use_certificate_chain_file(certificate_path);
		sslContext.use_private_key_file(key_path, asio::ssl::context::pem);
	}

	Server::~Server() {
		stop();
	}

	void Server::handleMessage(RemoteClient &client, std::string_view message) {
		if (onMessage)
			onMessage(client, message);
	}

	void Server::send(RemoteClient &client, std::string message, bool force) {
		if (message.empty())
			return;

		if (!force && client.isBuffering()) {
			SendBuffer &buffer = client.sendBuffer;
			auto lock = buffer.uniqueLock();
			buffer.bytes.insert(buffer.bytes.end(), message.begin(), message.end());
			return;
		}

		std::weak_ptr weak_client(std::static_pointer_cast<RemoteClient>(client.shared_from_this()));

		client.strand.post([this, weak_client, message = std::move(message)]() mutable {
			if (std::shared_ptr<RemoteClient> client = weak_client.lock())
				client->queue(std::move(message));
		});
	}

	void Server::accept() {
		INFO_("Accepting.");
		acceptor.async_accept([this](const asio::error_code &errc, asio::ip::tcp::socket socket) {
			if (errc) {
				ERROR_("Server accept: " << errc.message());
			} else {
				std::string ip = socket.remote_endpoint().address().to_string();
				auto client = std::make_shared<RemoteClient>(*this, ip, ++lastID, std::move(socket));
				allClients.insert(client);
				client->start();
				if (onAdd)
					onAdd(*client);
			}

			accept();
		});
	}

	void Server::run() {
		connected = true;
		asio::post(context, [this] { accept(); });
		context.run();
	}

	void Server::stop() {
		workGuard.reset();
		context.stop();
		if (onStop)
			onStop();
		game.reset();
	}

	bool Server::close(RemoteClient &client) {
		if (client.isClosed())
			return false;

		client.setClosed();

		try {
			client.socket.shutdown();
		} catch (const asio::system_error &err) {
			// Who really cares if SSL doesn't shut down properly?
			// Who decided that the client is worthy of a proper shutdown?
			ERROR_("Shutdown (" << client.ip << "): " << err.what() << " (" << err.code() << ')');
		}

		client.socket.lowest_layer().close();
		return true;
	}

	void Server::saveUserData() {
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

	std::shared_ptr<ServerPlayer> Server::loadPlayer(std::string_view username, std::string_view display_name) {
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
		overworld->add(player, overworld->randomLand);
		// overworld->add(player, Position(31, 82));
		// player->direction = Direction::Right;
		// overworld->add(player, Position(-25, -36));
		player->spawnPosition = player->getPosition();
		player->spawnRealmID = player->getRealm()->getID();
		INFO_("Setting player spawn realm ID to " << player->spawnRealmID);
		player->init(*game);
		player->onSpawn();
		const InventoryPtr inventory = player->getInventory(0);
		{
			auto inventory_lock = inventory->uniqueLock();
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_pickaxe"));
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_shovel"));
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_axe"));
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_hammer"));
			inventory->add(ItemStack::withDurability(*game, "base:item/iron_sword"));
			inventory->add(ItemStack(*game, "base:item/cave_entrance"));
		}
		game->addPlayer(player);
		return player;
	}

	Token Server::generateToken(const std::string &username) const {
		return computeSHA3_512<Token>(secret + '/' + username);
	}

	void Server::setupPlayer(RemoteClient &client) {
		auto player = client.getPlayer();
		auto realm = player->getRealm();
		INFO_("Setting up player");
		player->weakClient = std::static_pointer_cast<RemoteClient>(client.shared_from_this());
		player->notifyOfRealm(*realm);
		player->send(EntityMoneyChangedPacket(*player));
		auto guard = client.bufferGuard();
		client.send(SelfTeleportedPacket(realm->id, player->getPosition()));
		client.send(TimePacket(game->time));
		client.send(RecipeListPacket(CraftingRecipeRegistry::ID(), game->registry<CraftingRecipeRegistry>()));
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

	bool Server::validateUsername(std::string_view username) {
		if (username.empty() || 20 < username.size())
			return false;
		for (const char ch: username)
			if (!std::isalnum(ch) && ch != '_')
				return false;
		return true;
	}

	int Server::main(int argc, char **argv) {
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

		global_server = std::make_shared<Server>("::0", port, "private.crt", "private.key", secret, 2, 1024);

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGPIPE handler");

		auto stop_thread = std::thread([] {
			std::unique_lock lock(stopMutex);
			stopCV.wait(lock, [] { return !running.load(); });
			global_server->stop();
		});

		if (signal(SIGINT, +[](int) { running = false; stopCV.notify_all(); saveCV.notify_all(); }) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGINT handler");

		auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, std::make_pair(global_server, size_t(8))));

		global_server->onStop = [] {
			running = false;
			stopCV.notify_all();
			saveCV.notify_all();
		};

		const char *world_path = "world.db";
		if (4 <= argc)
			world_path = argv[3];

		const bool database_existed = std::filesystem::exists(world_path);
		game->openDatabase(world_path);
		global_server->game = game;

		size_t seed = 1621;
		if (std::filesystem::exists(".seed")) {
			seed = parseNumber<size_t>(strip(readFile(".seed")));
			INFO_("Using custom seed \e[1m" << seed << "\e[22m");
		}

		if (database_existed) {
			Timer timer{"ReadAll"};
			game->database.readAll();
			timer.stop();
			Timer::summary();
			Timer::clear();
			INFO_("Finished reading all data from database.");
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
				std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_TICK_PERIOD));
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

				if (running && save_period <= std::chrono::system_clock::now() - last_save) {
					INFO_("Autosaving...");
					game->tickingPaused = true;
					game->database.writeAll();
					game->tickingPaused = false;
					INFO_("Autosaved.");
					last_save = std::chrono::system_clock::now();
				}
			}
		});

		global_server->run();
		tick_thread.join();
		stop_thread.join();
		save_thread.join();

		return 0;
	}
}
