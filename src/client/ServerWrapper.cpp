#include "Log.h"
#include "client/ServerWrapper.h"
#include "game/ServerGame.h"
#include "game/SimulationOptions.h"
#include "net/Server.h"
#include "realm/Overworld.h"
#include "realm/ShadowRealm.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Shell.h"
#include "util/Timer.h"
#include "worldgen/Overworld.h"
#include "worldgen/ShadowRealm.h"

#include <fstream>
#include <random>

namespace Game3 {
	namespace {
		std::filesystem::path KEY_PATH{"localserver.key"};
		std::filesystem::path CERT_PATH{"localserver.crt"};
	}

	ServerWrapper::~ServerWrapper() {
		if (running)
			stop();

		if (threadActive) {
			runThread.join();
			threadActive = false;
		}
	}

	void ServerWrapper::runInThread(size_t overworld_seed) {
		runThread =	std::thread([this, overworld_seed] {
			pthread_setname_np(pthread_self(), "RunThread");
			threadActive = true;
			run(overworld_seed);
		});
	}

	void ServerWrapper::run(size_t overworld_seed) {
		if (running)
			throw std::runtime_error("Server is already running");

		const bool key_exists  = std::filesystem::exists(KEY_PATH);
		const bool cert_exists = std::filesystem::exists(CERT_PATH);

		if (key_exists != cert_exists)
			throw std::runtime_error("Exactly one of localserver.key, localserver.crt exists (should be both or neither)");

		if (!key_exists && !generateCertificate(CERT_PATH, KEY_PATH))
			throw std::runtime_error("Couldn't generate certificate/private key");

		std::string secret;

		if (std::filesystem::exists(".localsecret"))
			secret = readFile(".localsecret");
		else
			std::ofstream(".localsecret") << (secret = generateSecret(8));

		uint16_t port = 12255;

		running = true;
		server = std::make_shared<Server>("::0", port, CERT_PATH, KEY_PATH, secret, 2);

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't register SIGPIPE handler");

		std::thread stop_thread([this] {
			pthread_setname_np(pthread_self(), "StopThread");
			std::unique_lock lock(stopMutex);
			stopCV.wait(lock, [this] { return !running.load(); });
			server->stop();
		});

		game = std::dynamic_pointer_cast<ServerGame>(Game::create(Side::Server, std::make_pair(server, size_t(8))));

		server->onStop = [this] {
			running = false;
			stopCV.notify_all();
			saveCV.notify_all();
		};

		std::filesystem::path world_path = "localworld.db";
		const bool database_existed = std::filesystem::exists(world_path);

		game->openDatabase(world_path);
		server->game = game;

		if (overworld_seed == size_t(-1)) {
			std::random_device rng;
			overworld_seed = (uint64_t(rng()) << 32) | rng();
		}

		if (database_existed) {
			Timer timer{"ReadAll"};
			game->getDatabase().readAll();
			timer.stop();
			Timer::summary();
			Timer::clear();
			INFO_("Finished reading all data from database.");
		} else {
			RealmPtr realm = Realm::create<Overworld>(game, 1, Overworld::ID(), "base:tileset/monomap", overworld_seed);
			realm->outdoors = true;
			WorldGen::generateOverworld(realm, overworld_seed, {}, {{-1, -1}, {1, 1}}, true);
			game->addRealm(realm->id, realm);
		}

		if (!game->hasRealm(-1)) {
			RealmPtr shadow = Realm::create<ShadowRealm>(game, -1, ShadowRealm::ID(), "base:tileset/monomap", overworld_seed);
			shadow->outdoors = false;
			WorldGen::generateShadowRealm(shadow, overworld_seed, {}, {{-1, -1}, {1, 1}}, true);
			game->addRealm(shadow->id, shadow);
		}

		game->initEntities();
		game->initInteractionSets();

		std::thread tick_thread([&] {
			pthread_setname_np(pthread_self(), "TickThread");
			while (running) {
				if (!game->tickingPaused)
					game->tick();
				std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_TICK_PERIOD));
			}
		});

		std::mutex save_mutex;
		std::chrono::seconds save_period{120};

		std::thread save_thread([&] {
			pthread_setname_np(pthread_self(), "SaveThread");
			std::chrono::time_point last_save = std::chrono::system_clock::now();

			while (running) {
				std::unique_lock lock{save_mutex};
				saveCV.wait_for(lock, save_period, [&] {
					return !running || save_period <= std::chrono::system_clock::now() - last_save;
				});

				if (running && save_period <= std::chrono::system_clock::now() - last_save) {
					INFO_("Autosaving...");
					game->tickingPaused = true;
					game->getDatabase().writeAll();
					game->tickingPaused = false;
					INFO_("Autosaved.");
					last_save = std::chrono::system_clock::now();
				}
			}
		});

		server->run();
		tick_thread.join();
		stop_thread.join();
		save_thread.join();
	}

	void ServerWrapper::stop() {
		if (!game || !running)
			return;

		running = false;
		stopCV.notify_all();
		saveCV.notify_all();
	}

	bool ServerWrapper::isRunning() const {
		return running;
	}

	bool ServerWrapper::generateCertificate(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path) {
		runCommand("/usr/bin/openssl", {
			"req", "-x509", "-newkey", "rsa:4096", "-keyout", key_path.string(), "-out", certificate_path.string(), "-sha256", "-days", "36500", "-nodes", "-subj", "/C=/ST=/L=/O=/OU=/CN=",
		});

		return std::filesystem::exists(certificate_path) && std::filesystem::exists(key_path);
	}
}
