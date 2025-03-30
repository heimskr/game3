#include "util/Log.h"
#include "client/ServerWrapper.h"
#include "game/ServerGame.h"
#include "game/SimulationOptions.h"
#include "net/CertGen.h"
#include "net/DirectLocalClient.h"
#include "net/DirectRemoteClient.h"
#include "net/Server.h"
#include "realm/Overworld.h"
#include "realm/ShadowRealm.h"
#include "threading/ThreadContext.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "worldgen/Overworld.h"
#include "worldgen/ShadowRealm.h"

#include <fstream>
#include <random>

// #define REDIRECT_LOGS
#define CATCH_SERVERWRAPPER

#include "config.h"
#ifdef IS_FLATPAK
#define REDIRECT_LOGS
#endif

namespace Game3 {
	namespace {
		std::filesystem::path KEY_PATH{"localserver.key"};
		std::filesystem::path CERT_PATH{"localserver.crt"};
	}

	ServerWrapper::~ServerWrapper() {
		stop();
	}

	void ServerWrapper::runInThread(size_t overworld_seed) {
		stop();

		runThread =	std::thread([this, overworld_seed] {
			threadContext.rename("ServerRun");
			threadActive = true;
			run(overworld_seed);
		});
	}

	void ServerWrapper::run(size_t overworld_seed) {
#ifdef CATCH_SERVERWRAPPER
		try {
#endif
			if (running) {
				throw std::runtime_error("Server is already running");
			}

			const bool key_exists  = std::filesystem::exists(KEY_PATH);
			const bool cert_exists = std::filesystem::exists(CERT_PATH);

			if (key_exists != cert_exists) {
				throw std::runtime_error("Exactly one of localserver.key, localserver.crt exists (should be both or neither)");
			}

			if (!key_exists && !generateCertificate(CERT_PATH, KEY_PATH)) {
				throw std::runtime_error("Couldn't generate certificate/private key");
			}

			std::string secret;

			if (std::filesystem::exists(".localsecret")) {
				secret = readFile(".localsecret");
			} else {
				std::ofstream(".localsecret") << (secret = generateSecret(8));
			}

#ifndef __MINGW32__
			if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
				throw std::runtime_error("Couldn't register SIGPIPE handler");
			}

			logDataPipe.emplace();
#ifdef REDIRECT_LOGS
			logFDWrapper.init(logDataPipe->writeEnd(), {STDOUT_FILENO, STDERR_FILENO});
#endif
#endif

			port = 12255;
			running = true;
			server = Server::create("::0", port, CERT_PATH, KEY_PATH, secret, 2);

#ifndef __MINGW32__
			logThread = std::thread([this, fd = logDataPipe->readEnd(), control = logControlPipe.readEnd()] {
				threadContext.rename("ServerLog");
				fd_set fds{};
				FD_ZERO(&fds);
				FD_SET(control, &fds);
				FD_SET(fd, &fds);
				fd_set fds_copy = fds;
				const int nfds = std::max(fd, control) + 1;

				std::array<char, 8192> buffer;
				ssize_t bytes_read{};

				std::ofstream logstream("game3.log");

				while (select(nfds, &fds_copy, nullptr, nullptr, nullptr) != -1 || errno == EINTR) {
					if (FD_ISSET(control, &fds_copy)) {
						bytes_read = read(control, buffer.data(), buffer.size());

						if (bytes_read == -1) {
							throw std::runtime_error(std::format("Couldn't read from log control pipe ({})", errno));
						}

						if (0 < bytes_read && buffer[0] == 'r') {
							return;
						}
					}

					if (FD_ISSET(fd, &fds_copy)) {
						bytes_read = read(fd, buffer.data(), buffer.size());

						if (bytes_read == -1) {
							throw std::runtime_error(std::format("Couldn't read from log data pipe ({})", errno));
						}

						if (0 < bytes_read) {
							std::string_view text{buffer.data(), size_t(bytes_read)};
							logstream << text;
							if (onLog) {
								onLog(text);
							}
						}
					}

					fds_copy = fds;
				}
			});
#endif

			std::thread stop_thread([this] {
				threadContext.rename("ServerStop");
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
				INFO(2, "Finished reading all data from database.");
			} else {
				RealmPtr realm = Realm::create<Overworld>(game, 1, Overworld::ID(), "base:tileset/monomap", overworld_seed);
				realm->outdoors = true;
				game->addRealm(realm->id, realm);
				WorldGen::generateOverworld(realm, overworld_seed, {}, {{-1, -1}, {1, 1}}, true);
			}

			if (!game->hasRealm(-1)) {
				RealmPtr shadow = Realm::create<ShadowRealm>(game, -1, ShadowRealm::ID(), "base:tileset/monomap", overworld_seed);
				shadow->outdoors = false;
				game->addRealm(shadow->id, shadow);
				WorldGen::generateShadowRealm(shadow, overworld_seed, {}, {{-1, -1}, {1, 1}}, true);
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
						return !running || forceSave || save_period <= std::chrono::system_clock::now() - last_save;
					});

					if (running && (forceSave.exchange(false) || save_period <= std::chrono::system_clock::now() - last_save)) {
						INFO(2, "Saving...");
						game->tickingPaused = true;
						game->getDatabase().writeAll();
						game->tickingPaused = false;
						INFO(2, "Saved.");
						last_save = std::chrono::system_clock::now();
					}
				}
			});

			started = true;
			startCV.notify_all();
			server->run();
			tick_thread.join();
			stop_thread.join();
			save_thread.join();
#ifdef CATCH_SERVERWRAPPER
		} catch (const std::exception &error) {
			if (onError) {
				onError(error);
			} else {
				throw;
			}
		}
#endif
	}

	void ServerWrapper::stop() {
		directRemoteClient.reset();

		if (!game || !running) {
			return;
		}

		running = false;
		started = false;
		stopCV.notify_all();
		saveCV.notify_all();

		if (threadActive) {
			runThread.join();
			threadActive = false;
		}

		write(logControlPipe.writeEnd(), "r", 1);
		logThread.join();
		logFDWrapper.close();
		logDataPipe.reset();

		server.reset();
	}

	bool ServerWrapper::isRunning() const {
		return running;
	}

	bool ServerWrapper::waitUntilRunning(std::chrono::milliseconds timeout) {
		std::unique_lock lock{startMutex};

		if (timeout == std::chrono::milliseconds(0)) {
			startCV.wait(lock, [this] { return threadActive && isRunning(); });
		} else {
			startCV.wait_for(lock, timeout, [this] { return threadActive && isRunning(); });
		}

		return isRunning();
	}

	Token ServerWrapper::getOmnitoken() const {
		if (!server || !game) {
			throw std::runtime_error("Can't get omnitoken: server not available");
		}
		return game->getOmnitoken();
	}

	void ServerWrapper::save() {
		forceSave = true;
		saveCV.notify_all();
	}

	std::shared_ptr<DirectRemoteClient> ServerWrapper::getDirectRemoteClient(const std::shared_ptr<DirectLocalClient> &local) {
		if (directRemoteClient == nullptr) {
			assert(local != nullptr);
			assert(server != nullptr);
			directRemoteClient = std::make_shared<DirectRemoteClient>(server);
			directRemoteClient->setLocal(local);
			server->allClients.withUnique([this](std::unordered_set<GenericClientPtr> &all_clients) {
				all_clients.insert(directRemoteClient);
			});
		}

		return directRemoteClient;
	}

	bool ServerWrapper::generateCertificate(const std::filesystem::path &certificate_path, const std::filesystem::path &key_path) {
		try {
			generateCertPair(certificate_path, key_path);
		} catch (const std::runtime_error &error) {
			ERR("Certificate generation failed: {}", error.what());
			return false;
		}

		return std::filesystem::exists(certificate_path) && std::filesystem::exists(key_path);
	}
}
