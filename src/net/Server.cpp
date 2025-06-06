#include "util/Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "game/SimulationOptions.h"
#include "net/NetError.h"
#include "net/Server.h"
#include "net/GenericClient.h"
#include "net/RemoteClient.h"
#include "packet/EntityMoneyChangedPacket.h"
#include "packet/KnownItemsPacket.h"
#include "packet/RecipeListPacket.h"
#include "packet/SelfTeleportedPacket.h"
#include "packet/TimePacket.h"
#include "realm/Overworld.h"
#include "realm/ShadowRealm.h"
#include "recipe/CraftingRecipe.h"
#include "threading/ThreadContext.h"
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
	acceptor(context, asio::ip::tcp::endpoint(asio::ip::make_address(ip), port)),
	workGuard(asio::make_work_guard(context)) {
		if (thread_count < 1) {
			throw std::invalid_argument("Cannot instantiate a Server with a thread count of zero");
		}

		sslContext.use_certificate_chain_file(certificate_path.string());
		sslContext.use_private_key_file(key_path.string(), asio::ssl::context::pem);
	}

	Server::~Server() {
		stop();
	}

	void Server::handleMessage(GenericClient &client, std::string_view message) {
		if (onMessage) {
			onMessage(client, message);
		}
	}

	void Server::accept() {
		INFO(3, "Accepting.");
		acceptor.async_accept([this](const asio::error_code &errc, asio::ip::tcp::socket socket) {
			if (errc) {
				ERR("Server accept: {}", errc.message());
			} else {
				try {
					std::string ip = socket.remote_endpoint().address().to_string();
					auto client = std::make_shared<RemoteClient>(shared_from_this(), ip, ++lastID, std::move(socket));
					allClients.withUnique([&](auto &) {
						allClients.insert(client);
					});
					client->start();
					if (onAdd) {
						onAdd(*client);
					}
				} catch (const asio::system_error &err) {
					ERR("Server accept: {}", err.what());
				}
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
		// Game already stopped.
		if (!game) {
			return;
		}

		allClients.withUnique([&](auto &) {
			while (!allClients.empty()) {
				auto iter = allClients.begin();
				close(*iter);
				allClients.erase(iter);
			}
		});

		workGuard.reset();
		context.stop();
		game->stop();
		if (onStop) {
			onStop();
		}
		game.reset();
	}

	bool Server::close(GenericClientPtr client) {
		if (client->isClosed()) {
			return false;
		}

		client->setClosed();
		client->close();
		return true;
	}

	void Server::saveUserData() {
		auto lock = game->players.sharedLock();

		GameDB &database = game->getDatabase();
		database.writeUsers(game->players);

		for (const ServerPlayerPtr &player: game->players) {
			database.writeUser(*player);
		}
	}

	std::shared_ptr<ServerPlayer> Server::loadPlayer(std::string_view username, std::string_view display_name) {
		if (!validateUsername(username)) {
			return nullptr;
		}

		{
			Buffer buffer{game, Side::Server};
			if (game->getDatabase().readUser(std::string(username), nullptr, &buffer, nullptr)) {
				return ServerPlayer::fromBuffer(game, buffer);
			}
		}

		RealmPtr overworld = game->getRealm(1);

		auto player = Entity::create<ServerPlayer>();
		player->username = username;
		player->displayName = display_name;
		player->token = generateToken(player->username);
		overworld->add(player, overworld->randomLand);
		player->spawnPosition = player->getPosition();
		player->spawnRealmID = player->getRealm()->getID();
		INFO("Setting player spawn realm ID to {}", player->spawnRealmID);
		player->init(game);
		player->onSpawn();
		const InventoryPtr inventory = player->getInventory(0);
		{
			auto inventory_lock = inventory->uniqueLock();
			inventory->add(ItemStack::withDurability(game, "base:item/iron_pickaxe"));
			inventory->add(ItemStack::withDurability(game, "base:item/iron_shovel"));
			inventory->add(ItemStack::withDurability(game, "base:item/iron_axe"));
			inventory->add(ItemStack::withDurability(game, "base:item/iron_hammer"));
			inventory->add(ItemStack::withDurability(game, "base:item/iron_sword"));
			inventory->add(ItemStack::create(game, "base:item/cave_entrance"));
		}
		game->addPlayer(player);
		return player;
	}

	Token Server::generateToken(const std::string &username) const {
		return computeSHA3_512<Token>(secret + '/' + username);
	}

	void Server::setupPlayer(GenericClient &client) {
		auto player = client.getPlayer();
		auto realm = player->getRealm();
		INFO(2, "Setting up player");
		player->weakClient = client.weak_from_this();
		player->notifyOfRealm(*realm);
		auto guard = client.bufferGuard();
		client.send(make<EntityMoneyChangedPacket>(*player));
		client.send(make<SelfTeleportedPacket>(realm->id, player->getPosition()));
		client.send(make<TimePacket>(game->time));
		client.send(make<RecipeListPacket>(CraftingRecipeRegistry::ID(), game->registry<CraftingRecipeRegistry>(), game));
		client.send(make<KnownItemsPacket>(*player));
		auto lock = game->players.sharedLock();
		const auto packet = make<EntityPacket>(player);
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
		if (username.empty() || 20 < username.size()) {
			return false;
		}

		for (const char ch: username) {
			if (!std::isalnum(ch) && ch != '_')
				return false;
		}

		return true;
	}

	int Server::main(int argc, char **argv) {
		std::string secret;

		if (std::filesystem::exists(".secret")) {
			secret = readFile(".secret");
		} else {
			std::ofstream(".secret") << (secret = generateSecret(8));
		}

		uint16_t port = 12255;
		if (3 <= argc) {
			try {
				port = parseNumber<uint16_t>(argv[2]);
			} catch (const std::invalid_argument &) {}
		}

		global_server = Server::create("::0", port, "private.crt", "private.key", secret, 2);

#ifndef __MINGW32__
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
			throw std::runtime_error("Couldn't register SIGPIPE handler");
		}
#endif

		std::thread stop_thread([] {
			threadContext.rename("ServerStop");
			std::unique_lock lock(stopMutex);
			stopCV.wait(lock, [] { return !running.load(); });
			global_server->stop();
		});

		if (signal(SIGINT, +[](int) { running = false; stopCV.notify_all(); saveCV.notify_all(); }) == SIG_ERR) {
			throw std::runtime_error("Couldn't register SIGINT handler");
		}

		auto game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, std::make_pair(global_server, size_t(8))));

		global_server->onStop = [] {
			running = false;
			stopCV.notify_all();
			saveCV.notify_all();
		};

		const char *world_path = "world.game3";
		if (4 <= argc) {
			world_path = argv[3];
		}

		const bool database_existed = std::filesystem::exists(world_path);
		game->openDatabase(world_path);
		global_server->game = game;

		size_t seed = 1621;
		if (std::filesystem::exists(".seed")) {
			seed = parseNumber<size_t>(trim(readFile(".seed")));
			INFO("Using custom seed \e[1m{}\e[22m", seed);
		}

		if (database_existed) {
			Timer timer{"ReadAll"};
			game->getDatabase().readAll();
			timer.stop();
			Timer::summary();
			Timer::clear();
			INFO(2, "Finished reading all data from database.");
		} else {
			RealmPtr realm = Realm::create<Overworld>(game, 1, Overworld::ID(), "base:tileset/monomap", seed);
			realm->outdoors = true;
			game->addRealm(realm->id, realm);
			WorldGen::generateOverworld(realm, seed, {}, {{-1, -1}, {1, 1}}, true);
		}

		if (!game->hasRealm(-1)) {
			RealmPtr shadow = Realm::create<ShadowRealm>(game, -1, ShadowRealm::ID(), "base:tileset/monomap", seed);
			shadow->outdoors = false;
			game->addRealm(shadow->id, shadow);
			WorldGen::generateShadowRealm(shadow, seed, {}, {{-1, -1}, {1, 1}}, true);
		}

		game->initEntities();
		game->initInteractionSets();

		std::thread tick_thread([&] {
			threadContext.rename("ServerTick");
			while (running) {
				if (!game->tickingPaused) {
					game->tick();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_TICK_PERIOD));
			}
		});

		std::mutex save_mutex;
		std::chrono::seconds save_period{120};

		std::thread save_thread([&] {
			threadContext.rename("ServerSave");
			std::chrono::time_point last_save = std::chrono::system_clock::now();

			while (running) {
				std::unique_lock lock{save_mutex};
				saveCV.wait_for(lock, save_period, [&] {
					return !running || save_period <= std::chrono::system_clock::now() - last_save;
				});

				if (running && save_period <= std::chrono::system_clock::now() - last_save) {
					game->tickingPaused = true;
					game->getDatabase().writeAll();
					game->tickingPaused = false;
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

	void Server::send(GenericClient &client, std::string message, bool force) {
		client.send(std::move(message), force);
	}
}
