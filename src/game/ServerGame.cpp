#include "entity/Animal.h"
#include "entity/EntityFactory.h"
#include "entity/ItemEntity.h"
#include "entity/ServerPlayer.h"
#include "error/IncompatibleError.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "net/RemoteClient.h"
#include "net/Server.h"
#include "packet/ChatMessageSentPacket.h"
#include "packet/CommandResultPacket.h"
#include "packet/DestroyEntityPacket.h"
#include "packet/DestroyTileEntityPacket.h"
#include "packet/EntityChangingRealmsPacket.h"
#include "packet/EntityMovedPacket.h"
#include "packet/FluidUpdatePacket.h"
#include "packet/InventoryPacket.h"
#include "packet/TileEntityPacket.h"
#include "packet/TileUpdatePacket.h"
#include "packet/TimePacket.h"
#include "realm/Overworld.h"
#include "realm/ShadowRealm.h"
#include "statuseffect/StatusEffectFactory.h"
#include "threading/ThreadContext.h"
#include "util/Cast.h"
#include "util/Demangle.h"
#include "util/Explosion.h"
#include "util/Log.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/ShadowRealm.h"

#include <iomanip>
#include <random>

namespace Game3 {
	ServerGame::ServerGame(const std::shared_ptr<Server> &server, size_t pool_size):
		weakServer(server),
		pool(pool_size) {
		pool.start();
	}

	ServerGame::~ServerGame() {
		INFO(3, "\e[31m~ServerGame\e[39m({})", reinterpret_cast<void *>(this));
	}

	void ServerGame::init() {
		database = std::make_unique<GameDB>(getSelf());
	}

	void ServerGame::stop() {
		pool.join();
		if (databaseValid) {
			INFO("Saving realms and users...");
			assert(database);
			database->writeAllRealms();
			database->writeUsers(players);
			SUCCESS(2, "Saved realms and users.");
		}
		Timer::summary();
		Timer::clear();
	}

	double ServerGame::getFrequency() const {
		return SERVER_TICK_FREQUENCY;
	}

	void ServerGame::addEntityFactories() {
		Game::addEntityFactories();
		add(EntityFactory::create<ServerPlayer>());
	}

	bool ServerGame::tick() {
		if (!Game::tick()) {
			return false;
		}

		std::unordered_map<Player *, std::unique_ptr<BufferGuard>> guards;
		guards.reserve(players.size());

		for (const auto &player: players) {
			if (auto client = player->toServer()->weakClient.lock()) {
				guards.emplace(player.get(), client->bufferGuard());
			}
		}

		for (const auto &[weak_client, packet]: packetQueue.steal()) {
			if (auto client = weak_client.lock()) {
				handlePacket(*client, *packet);
			}
		}

		// const size_t max_jobs = realms.size() * 2;

		for (auto &[id, realm]: realms) {
			// if (max_jobs <= pool.jobCount()) {
			// 	break;
			// }
			// pool.add([weak_realm = std::weak_ptr(realm), delta = delta](ThreadPool &, size_t) {
			// 	if (RealmPtr realm = weak_realm.lock()) {
					realm->tick(delta);
			// 	}
			// });
		}

		std::shared_ptr<TimePacket> time_packet;
		timeSinceTimeUpdate += delta;
		if (10. <= timeSinceTimeUpdate) {
			time_packet = make<TimePacket>(time);
			timeSinceTimeUpdate = 0.;
		}

		for (const ServerPlayerPtr &player: players) {
			player->ticked = false;

			if (time_packet) {
				player->send(time_packet);
			}

			if (player->inventoryUpdated) {
				player->send(make<InventoryPacket>(player->getInventory(0)));
				player->inventoryUpdated = false;
			}
		}

		for (const auto &weak_player: playerRemovalQueue.steal()) {
			if (auto player = weak_player.lock()) {
				guards.erase(player.get());
				remove(player);
				player->toServer()->weakClient.reset();
				player->clearQueues();

				for (const auto &[id, realm]: realms) {
					realm->eviscerate(player, true);
				}

				if (auto count = player.use_count(); count != 1) {
					WARN("Player {} ref count: {} (should be 1). Current realm: {} (realmID) or {} (getRealm()->id)", reinterpret_cast<void *>(player.get()), count, player->realmID, player->getRealm()->id);
				}
			}
		}

		lastGarbageCollection += delta;
		if (GARBAGE_COLLECTION_TIME <= lastGarbageCollection) {
			garbageCollect();
			lastGarbageCollection = 0.f;
		}

		return true;
	}

	void ServerGame::garbageCollect() {
		auto lock = players.sharedLock();

		for (const auto &player: players) {
			{
				auto shared_lock = player->knownEntities.sharedLock();
				if (player->knownEntities.empty()) {
					continue;
				}
			}
			auto unique_lock = player->knownEntities.uniqueLock();
			std::vector<std::weak_ptr<Entity>> to_remove;
			to_remove.reserve(player->knownEntities.size() / 4);
			for (const auto &weak_entity: player->knownEntities) {
				if (!weak_entity.lock()) {
					to_remove.push_back(weak_entity);
				}
			}
			for (const auto &weak_entity: to_remove) {
				player->knownEntities.erase(weak_entity);
			}
		}
	}

	void ServerGame::broadcastTileUpdate(RealmID realm_id, Layer layer, const Position &position, TileID tile_id) {
		broadcast({position, realms.at(realm_id), nullptr}, make<TileUpdatePacket>(realm_id, layer, position, tile_id));
	}

	void ServerGame::broadcastFluidUpdate(RealmID realm_id, const Position &position, FluidTile tile) {
		broadcast({position, realms.at(realm_id), nullptr}, make<FluidUpdatePacket>(realm_id, position, tile));
	}

	void ServerGame::queuePacket(std::shared_ptr<GenericClient> client, std::shared_ptr<Packet> packet) {
		packetQueue.emplace(std::move(client), std::move(packet));
	}

	void ServerGame::runCommand(GenericClient &client, const std::string &command, GlobalID command_id) {
		auto [success, message] = commandHelper(client, command);
		client.send(make<CommandResultPacket>(command_id, success, std::move(message)));
	}

	void ServerGame::entityChangingRealms(Entity &entity, const RealmPtr &new_realm, const Position &new_position) {
		const auto changing_packet = make<EntityChangingRealmsPacket>(entity.getGID(), new_realm->id, new_position);
		const auto moved_packet = make<EntityMovedPacket>(entity);
		moved_packet->arguments.position = new_position;
		moved_packet->arguments.isTeleport = true;

		auto lock = players.sharedLock();
		for (const auto &player: players) {
			if (player->knowsRealm(new_realm->id)) {
				player->send(moved_packet);
			} else {
				player->send(changing_packet);
			}
		}
	}

	void ServerGame::entityTeleported(Entity &entity, MovementContext context) {
		if (entity.spawning) {
			return;
		}

		const auto packet = make<EntityMovedPacket>(entity);
		packet->arguments.isTeleport = context.isTeleport;

		// Actual teleportation (rather than regular movement between adjacent tiles) should be instant.
		if (context.isTeleport) {
			packet->arguments.adjustOffset = false;
		}

		if (auto cast_player = dynamic_cast<Player *>(&entity)) {
			if (context.excludePlayer != cast_player->getGID()) {
				cast_player->send(packet);
			}
			auto lock = players.sharedLock();
			for (const auto &player: players) {
				if (player.get() != cast_player && player->getRealm() && player->canSee(entity)) {
					if (auto client = player->toServer()->weakClient.lock()) {
						client->send(packet);
					}
				}
			}
			return;
		}

		auto lock = players.sharedLock();
		for (const auto &player: players) {
			if (player->getRealm() && player->canSee(entity) && player->getGID() != context.excludePlayer) {
				if (auto client = player->toServer()->weakClient.lock()) {
					client->send(packet);
				}
			}
		}
	}

	void ServerGame::entityDestroyed(const Entity &entity) {
		if (!entity.shouldBroadcastDestruction()) {
			return;
		}

		const auto packet = make<DestroyEntityPacket>(entity, false);
		ServerPtr server = weakServer.lock();
		assert(server != nullptr);
		auto &clients = server->getClients();
		std::shared_lock lock = clients.sharedLock();
		for (const auto &client: clients) {
			if (PlayerPtr player = client->getPlayer(); player && player == entity.weakExcludedPlayer.lock()) {
				continue;
			}
			client->send(packet);
		}
	}

	void ServerGame::tileEntitySpawned(const TileEntityPtr &tile_entity) {
		const auto packet = make<TileEntityPacket>(tile_entity);
		auto realm = tile_entity->getRealm();
		realm->updateNeighbors(tile_entity->getPosition(), Layer::Submerged);
		realm->updateNeighbors(tile_entity->getPosition(), Layer::Objects);
		ChunkRange(tile_entity->getChunk()).iterate([&](ChunkPosition chunk_position) {
			if (auto entities = realm->getEntities(chunk_position)) {
				auto lock = entities->sharedLock();
				for (const WeakEntityPtr &weak_entity: *entities) {
					if (EntityPtr entity = weak_entity.lock(); entity && entity->isPlayer()) {
						safeDynamicCast<ServerPlayer>(entity)->send(packet);
					}
				}
			}
		});
	}

	void ServerGame::tileEntityDestroyed(const TileEntity &tile_entity) {
		const auto packet = make<DestroyTileEntityPacket>(tile_entity);
		auto server = weakServer.lock();
		assert(server);
		auto &clients = server->getClients();
		auto lock = clients.sharedLock();
		for (const GenericClientPtr &client: clients) {
			client->send(packet);
		}
	}

	void ServerGame::remove(const ServerPlayerPtr &player) {
		{
			auto set_lock = players.uniqueLock();
			players.erase(player);
			auto map_lock = playerMap.uniqueLock();
			playerMap.erase(player->username);
		}
		player->destroy();
	}

	void ServerGame::addPlayer(const ServerPlayerPtr &player) {
		assert(!player->username.empty());
		auto set_lock = players.uniqueLock();
		players.insert(player);
		auto map_lock = playerMap.uniqueLock();
		playerMap.emplace(player->username, player);
	}

	bool ServerGame::hasPlayer(const std::string &username) const {
		auto lock = playerMap.sharedLock();
		return playerMap.contains(username);
	}

	void ServerGame::queueRemoval(const ServerPlayerPtr &player) {
		playerRemovalQueue.push(player);
	}

	void ServerGame::openDatabase(std::filesystem::path path) {
		assert(database);
		database->open(std::move(path));
		if (int64_t compatibility = database->getCompatibility(); compatibility != 0 && compatibility != INT64_MIN) {
			ERR("Incompatible by {}", compatibility);
			throw IncompatibleError(compatibility);
		} else {
			SUCCESS("Compatible.");
		}
		databaseValid = true;
	}

	void ServerGame::broadcast(const PacketPtr &packet, bool include_non_players) {
		if (include_non_players) {
			std::shared_ptr<Server> server = getServer();
			auto &clients = server->getClients();
			auto lock = clients.sharedLock();
			for (const GenericClientPtr &client: clients) {
				client->send(packet);
			}
		} else {
			auto lock = players.sharedLock();
			for (const ServerPlayerPtr &player: players) {
				player->send(packet);
			}
		}
	}

	void ServerGame::releasePlayer(const std::string &username, const Place &place) {
		assert(database);
		auto lock = playerMap.sharedLock();
		if (auto iter = playerMap.find(username); iter != playerMap.end()) {
			iter->second->teleport(place.position, place.realm, MovementContext{.isTeleport = true});
		} else {
			lock.unlock();
			database->writeReleasePlace(username, place);
		}
	}

	void ServerGame::setRule(const std::string &key, ssize_t value) {
		auto lock = gameRules.uniqueLock();
		gameRules[key] = value;
	}

	std::optional<ssize_t> ServerGame::getRule(const std::string &key) const {
		auto lock = gameRules.sharedLock();
		if (auto iter = gameRules.find(key); iter != gameRules.end()) {
			return std::make_optional(iter->second);
		}
		return std::nullopt;
	}

	void ServerGame::removeRealm(RealmPtr realm) {
		assert(database);
		Game::removeRealm(realm);

		{
			RealmPtr shadow_realm = getRealm(-1);
			// Teleport players to the shadow realm.
			auto lock = realm->players.uniqueLock();
			for (const auto &weak_player: realm->players) {
				if (PlayerPtr player = weak_player.lock()) {
					player->teleport(Position{32, 32}, shadow_realm);
				}
			}
		}

		database->deleteRealm(realm);
	}

	bool ServerGame::compareToken(Token check) {
		if (check == omnitoken) {
			omnitoken = generateRandomToken();
			return true;
		}

		return false;
	}

	Token ServerGame::getOmnitoken() const {
		return omnitoken;
	}

	bool ServerGame::initialWorldgen(size_t overworld_seed) {
		if (hasRealm(1)) {
			return false;
		}

		auto self = getSelf();
		RealmPtr realm = Realm::create<Overworld>(self, 1, Overworld::ID(), "base:tileset/monomap", overworld_seed);
		realm->outdoors = true;
		addRealm(realm->id, realm);
		WorldGen::generateOverworld(realm, overworld_seed, {}, {{-1, -1}, {1, 1}}, true);

		if (!hasRealm(-1)) {
			RealmPtr shadow = Realm::create<ShadowRealm>(self, -1, ShadowRealm::ID(), "base:tileset/monomap", overworld_seed);
			shadow->outdoors = false;
			addRealm(shadow->id, shadow);
			WorldGen::generateShadowRealm(shadow, overworld_seed, {}, {{-1, -1}, {1, 1}}, true);
		}

		return true;
	}

	void ServerGame::handlePacket(GenericClient &client, Packet &packet) {
		packet.handle(getSelf(), client);
	}

	std::tuple<bool, std::string> ServerGame::commandHelper(GenericClient &client, const std::string &command) {
		if (command.empty()) {
			return {false, "Command is empty."};
		}

		const auto words = split(command, " ", false);
		const auto &first = words.at(0);
		auto player = client.getPlayer();

		// Change this check if there are any miscellaneous commands implemented later that don't require a player.
		if (!player) {
			return {false, "No player."};
		}

		try {
			if (command[0] == ':') {
				std::string_view message = trim(std::string_view(command).substr(1));

				if (message.empty() || message == " ") {
					return {true, ""};
				}

				if (message[0] == ' ') {
					message.remove_prefix(1);
				}

				INFO("[{}] {}", player->username, message);
				broadcast(make<ChatMessageSentPacket>(player->getGID(), std::string(message)), true);
				return {true, ""};
			}

			if (command[0] == '!') {
				// Place a tile entity in front of the player
				RealmPtr realm = player->getRealm();
				if (!realm) {
					return {false, "No realm."};
				}
				const Position position = player->getPosition() + player->getDirection();
				boost::json::value json = boost::json::parse(trim(std::string_view(command).substr(1)));
				if (realm->hasTileEntityAt(position)) {
					return {false, "Tile entity already present."};
				}
				GamePtr game = realm->getGame();
				TileEntityPtr tile_entity = TileEntity::fromJSON(game, json);
				tile_entity->position = position;
				tile_entity->setRealm(realm);
				tile_entity->init(*game);
				realm->addToMaps(tile_entity);
				realm->attach(tile_entity);
				tile_entity->onSpawn();
				return {true, ""};
			}

			if (first == "give") {
				if (words.size() < 2) {
					return {false, "Not enough arguments."};
				}

				ItemCount count = 1;
				if (3 <= words.size()) {
					try {
						count = parseNumber<ItemCount>(words.at(2));
					} catch (const std::invalid_argument &) {
						return {false, "Invalid count."};
					}
				}

				boost::json::value data;
				if (3 < words.size()) {
					try {
						data = boost::json::parse(join(std::span(words.begin() + 3, words.end()), " "));
					} catch (const std::exception &err) {
						ERR("{}", err.what());
						return {false, "Couldn't parse data as JSON."};
					}
				}

				std::string item_name(words.at(1));
				const size_t colon = item_name.find(':');

				if (colon == item_name.npos) {
					item_name = "base:item/" + std::string(item_name);
				}

				if (auto item = (*itemRegistry)[Identifier(item_name)]) {
					player->give(ItemStack::create(shared_from_this(), item, count, std::move(data)));
					return {true, "Gave " + std::to_string(count) + " x " + item->name};
				}

				return {false, "Unknown item: " + item_name};
			}

			if (first == "heldL" || first == "heldR") {
				if (words.size() != 2) {
					return {false, "Invalid number of arguments."};
				}
				Slot slot = -1;
				try {
					slot = parseNumber<Slot>(words.at(1));
				} catch (const std::invalid_argument &) {
					return {false, "Invalid slot."};
				}
				if (first == "heldL") {
					player->setHeldLeft(slot);
				} else {
					player->setHeldRight(slot);
				}
				return {true, "Set held slot to " + std::to_string(slot)};
			}

			if (first == "h") {
				runCommand(client, "heldL 0", threadContext.rng());
				runCommand(client, "heldR 1", threadContext.rng());
			}

			if (first == "go") {
				if (words.size() != 3) {
					return {false, "Invalid number of arguments."};
				}

				Index row = 0;
				Index column = 0;
				try {
					row = parseLong(words.at(1));
					column = parseLong(words.at(2));
				} catch (const std::invalid_argument &) {
					return {false, "Invalid coordinates."};
				}

				player->teleport(Position(row, column));
				return {true, "Teleported to " + static_cast<std::string>(Position(row, column))};
			}

			if (first == "counter") {
				const auto chunk = player->getChunk();
				const auto counter = player->getRealm()->tileProvider.getUpdateCounter(chunk);
				return {true, "Counter for chunk " + static_cast<std::string>(chunk) + ": " + std::to_string(counter)};
			}

			if (first == "moving") {
				std::stringstream ss;
				if (player->isMoving()) {
					std::vector<const char *> moves;
					if (player->movingUp) moves.push_back("up");
					if (player->movingDown) moves.push_back("down");
					if (player->movingLeft) moves.push_back("left");
					if (player->movingRight) moves.push_back("right");
					ss << "Player is moving ";
					bool first = true;
					for (const char *move: moves) {
						if (first) {
							first = false;
						} else {
							ss << ", ";
						}
						ss << move;
					}
					ss << ". Offset: " << player->offset.x << ", " << player->offset.y << ", " << player->offset.z;
				} else {
					ss << "Player isn't moving. Offset: " << player->offset.x << ", " << player->offset.y << ", " << player->offset.z;
				}
				return {true, std::move(ss).str()};
			}

			if (isAny(first, "submerge", "bedrock", "soil", "veg", "vegetation", "flooring", "snow", "objects", "obj")) {
				if (words.size() != 2) {
					return {false, "Invalid number of arguments."};
				}
				std::string_view word = words.at(1);
				Identifier identifier = word.find(':') == std::string_view::npos? "base:tile/" + std::string(word) : word;
				Layer layer = Layer::Objects;
				if (first == "submerge") {
					layer = Layer::Submerged;
				} else if (first == "bedrock") {
					layer = Layer::Bedrock;
				} else if (first == "soil") {
					layer = Layer::Soil;
				} else if (first == "veg" || first == "vegetation") {
					layer = Layer::Vegetation;
				} else if (first == "floor" || first == "flooring") {
					layer = Layer::Flooring;
				} else if (first == "snow") {
					layer = Layer::Snow;
				}
				player->getRealm()->setTile(layer, player->getPosition(), identifier);
				return {true, ""};
			}

			if (first == "saveall") {
				INFO("Writing...");
				assert(database);
				tickingPaused = true;
				database->writeAll();
				tickingPaused = false;
				INFO("Writing done.");
				return {true, "Wrote all data."};
			}

			if (first == "pos") {
				INFO("Player {} position: {}", player->getGID(), player->getPosition());
				INFO("Player {} chunk position: {}", player->getGID(), player->getChunk());
				return {true, "Position = " + std::string(player->getPosition()) + ", chunk position = " + std::string(player->getChunk())};
			}

			if (first == "pm") {
				if (1 < words.size()) {
					player->getRealm()->remakePathMap(player->getChunk());
				}

				TileProvider &provider = player->getRealm()->tileProvider;
				auto &path_chunk = provider.getPathChunk(player->getChunk());
				auto lock = path_chunk.sharedLock();
				size_t walkables = 0;
				for (size_t y = 0; y < CHUNK_SIZE; ++y) {
					for (size_t x = 0; x < CHUNK_SIZE; ++x) {
						const auto walkable = path_chunk[y * CHUNK_SIZE + x];
						std::cerr << (walkable? "\u2588" : "\u2591");
						walkables += walkable;
					}
					std::cerr << '\n';
				}
				return {true, "Walkable: " + std::to_string(walkables) + " / " + std::to_string(CHUNK_SIZE * CHUNK_SIZE)};
			}

			if (first == "stop") {
				if (player->username != "heimskr") {
					return {false, "No thanks."};
				}
				auto server = weakServer.lock();
				if (!server) {
					return {false, "Couldn't lock server."};
				}
				server->stop();
				return {true, "Stopped server."};
			}

			if (first == "say") {
				std::string_view message = std::string_view(command).substr(first.size() + 1);
				INFO("[{}] {}", player->username, message);
				broadcast(make<ChatMessageSentPacket>(player->getGID(), std::string(message)), true);
				return {true, ""};
			}

			if (first == "goto") {
				auto server = weakServer.lock();
				if (!server) {
					return {false, "Couldn't lock server."};
				}

				if (words.size() != 2) {
					return {false, "Invalid number of arguments."};
				}

				ServerPlayerPtr other_player;
				{
					const std::string other_username(words.at(1));
					auto lock = playerMap.sharedLock();
					if (auto iter = playerMap.find(other_username); iter != playerMap.end()) {
						other_player = iter->second;
					} else {
						return {false, "Couldn't find player " + other_username + "."};
					}
				}

				player->teleport(other_player->getPosition(), other_player->getRealm(), {.isTeleport = true});

				return {true, "Teleported."};
			}

			if (first == "realm") {
				if (words.size() != 2) {
					return {false, "Invalid number of arguments."};
				}

				RealmID id{};

				try {
					id = parseNumber<RealmID>(words.at(1));
				} catch (const std::invalid_argument &) {
					return {false, "Invalid number."};
				}

				RealmPtr realm;

				try {
					realm = getRealm(id);
				} catch (const std::out_of_range &) {
					return {false, "Realm not found."};
				}

				player->teleport(player->getPosition(), realm, {.isTeleport = true});

				return {true, "Teleported."};
			}

			if (first == "texture") {
				if (words.size() != 1 && words.size() != 2) {
					return {false, "Invalid number of arguments."};
				}

				Identifier choice;

				if (words.size() < 2) {
					choice = "base:entity/player";
				} else if (words[1].find(':') != std::string_view::npos) {
					choice = words[1];
				} else {
					choice = Identifier("base:entity/" + std::string(words[1]));
				}

				auto &textures = registry<EntityTextureRegistry>();
				if (!textures.contains(choice)) {
					return {false, "Invalid entity texture."};
				}

				player->changeTexture(choice);
				player->increaseUpdateCounter();
				player->sendToVisible();
				player->sendTo(*player->toServer()->getClient());
				return {true, "Changed texture to " + choice.str() + "."};
			}

			if (first == "unhold") {
				player->setHeldLeft(-1);
				player->setHeldRight(-1);
				return {true, "Unequipped items."};
			}

			if (first == "online") {
				std::set<std::string> display_names;
				players.withShared([&](const auto &players) {
					for (const auto &iterated_player: players) {
						display_names.insert(iterated_player->displayName);
					}
				});
				return {true, "Online players: " + join(display_names, ", ")};
			}

			if (first == "entities" || first == "ents") {
				auto lock = allAgents.sharedLock();
				std::vector<EntityPtr> entities;

				auto has_arg = [&](const char *arg) {
					for (const std::string_view &word: words) {
						if (word == arg) {
							return true;
						}
					}

					return false;
				};

				const bool exclude_both = has_arg("-ai");
				const bool exclude_animals = exclude_both || has_arg("-a");
				const bool exclude_items = exclude_both || has_arg("-i");

				for (const auto &[gid, weak_agent]: allAgents) {
					if (AgentPtr agent = weak_agent.lock()) {
						if (GlobalID agent_gid = agent->getGID(); gid != agent_gid) {
							WARN("Agent {} is stored in allAgents with key {}", agent_gid, gid);
						}

						if (auto entity = std::dynamic_pointer_cast<Entity>(agent)) {
							if (!exclude_animals || !std::dynamic_pointer_cast<Animal>(entity)) {
								if (!exclude_items || !std::dynamic_pointer_cast<ItemEntity>(entity)) {
									entities.push_back(entity);
								}
							}
						}
					}
				}

				std::sort(entities.begin(), entities.end(), [](const EntityPtr &left, const EntityPtr &right) {
					Entity &left_ref = *left;
					Entity &right_ref = *right;
					const std::string left_demangled = DEMANGLE(left_ref);
					const std::string right_demangled = DEMANGLE(right_ref);

					if (left_demangled < right_demangled) {
						return true;
					}

					if (left_demangled > right_demangled) {
						return false;
					}

					const std::string left_name = left->getName();
					const std::string right_name = right->getName();

					if (left_name < right_name) {
						return true;
					}

					if (left_name > right_name) {
						return false;
					}

					return left->getGID() < right->getGID();
				});

				INFO("Entity count: {}", entities.size());

				for (const EntityPtr &entity: entities) {
					Entity &entity_ref = *entity;

					if (entity->isPlayer()) {
						INFO("\e[1m({}) \e[22;2m {:<20}\e[22m {} \e[32m{}\e[39m Realm \e[31m{}\e[39m Position \e[33m{}\e[39m",
							safeDynamicCast<Player>(entity)->getUsername(), entity->getGID(), DEMANGLE(entity_ref), entity->getName(), entity->getRealm()->getID(), entity->getPosition());
					} else {
						INFO("\e[2m {:<20}\e[22m {} \e[32m{}\e[39m Realm \e[31m{}\e[39m Position \e[33m{}\e[39m",
							entity->getGID(), DEMANGLE(entity_ref), entity->getName(), entity->getRealm()->getID(), entity->getPosition());
					}
				}

				return {true, ""};
			}

			if (first == "tiles") {
				RealmPtr realm = player->getRealm();
				Tileset &tileset = realm->getTileset();
				for (const Layer layer: allLayers) {
					if (auto tile = realm->tryTile(layer, player->position)) {
						INFO("{} \e[2m→\e[22m {} \e[2m/\e[22m {}", getIndex(layer), *tile, tileset[*tile]);
					}
				}
				return {true, ""};
			}

			if (first == "regen") {
				if (player->username != "heimskr") {
					return {false, "No thanks."};
				}

				if (words.size() != 3) {
					return {false, "Incorrect parameter count."};
				}

				ChunkPosition chunk_position;
				try {
					chunk_position.x = parseNumber<ChunkPosition::IntType>(words[1]);
					chunk_position.y = parseNumber<ChunkPosition::IntType>(words[2]);
				} catch (const std::invalid_argument &) {
					return {false, "Couldn't parse chunk position."};
				}

				if (player->getChunk() == chunk_position) {
					player->getRealm()->generateChunk(chunk_position);
					return {true, "Regenerated chunk."};
				}

				return {false, "Incorrect chunk position."};
			}

			if (first == "rule") {
				if (words.size() != 3) {
					return {false, "Incorrect parameter count."};
				}

				std::string value_word{};
				if (words[2] == "true") {
					value_word = "1";
				} else if (words[2] == "false") {
					value_word = "0";
				} else {
					value_word = words[2];
				}

				ssize_t value{};
				try {
					value = parseNumber<ssize_t>(value_word);
				} catch (const std::invalid_argument &) {
					return {false, "Couldn't parse value."};
				}

				setRule(std::string(words[1]), value);
				return {true, "Rule set."};
			}

			if (first == "spawn") {
				if (words.size() != 2) {
					return {false, "Incorrect parameter count."};
				}

				Identifier entity_id;
				std::string_view name = words[1];

				if (name.find(':') == std::string::npos) {
					entity_id = Identifier("base:entity/" + std::string(name));
				} else {
					entity_id = name;
				}

				auto factory = registry<EntityFactoryRegistry>().maybe(entity_id);
				if (!factory) {
					return {false, "Unknown entity type."};
				}

				RealmPtr realm = player->getRealm();
				EntityPtr entity = (*factory)(shared_from_this());
				entity->spawning = true;
				entity->setRealm(realm);
				realm->queueEntityInit(std::move(entity), player->getPosition());
				return {true, ""};
			}

			if (first == "set_money") {
				if (words.size() != 2) {
					return {false, "Incorrect parameter count."};
				}

				MoneyCount value{};
				try {
					value = parseNumber<MoneyCount>(words.at(1));
				} catch (const std::invalid_argument &) {
					return {false, "Couldn't parse value."};
				}

				player->setMoney(value);
				return {true, ""};
			}

			if (first == "infinifluid" || first == "inff") {
				if (words.size() != 2) {
					return {false, "Incorrect parameter count."};
				}

				Identifier fluid_id;
				std::string_view name = words[1];

				if (name.contains(':')) {
					fluid_id = name;
				} else {
					fluid_id = Identifier("base:fluid/" + std::string(name));
				}

				FluidPtr fluid = getFluid(fluid_id);
				if (!fluid) {
					return {false, "No such fluid."};
				}

				player->getPlace().setFluid({static_cast<FluidID>(fluid->registryID), FluidTile::FULL, true});
				return {true, ""};
			}

			if (first == "heal") {
				if (words.size() != 1) {
					return {false, "Incorrect parameter count."};
				}

				player->setHealth(player->getMaxHealth());
				player->setStatusEffects({});
				return {true, ""};
			}

			if (first == "status") {
				if (words.size() != 2) {
					return {false, "Incorrect parameter count."};
				}

				Identifier status_id;
				std::string_view name = words[1];

				if (name.contains(':')) {
					status_id = name;
				} else {
					status_id = Identifier("base:statuseffect/" + std::string(name));
				}

				if (auto factory = registry<StatusEffectFactoryRegistry>().maybe(status_id)) {
					player->inflictStatusEffect((*factory)(), true);
					return {true, ""};
				}

				return {false, "No such status effect."};
			}

			if (first == "setspawn") {
				if (words.size() == 1) {
					player->spawnPosition = player->getPosition();
					return {true, ""};
				}

				if (words.size() != 3) {
					return {false, "Incorrect parameter count."};
				}

				try {
					auto row = parseNumber<Index>(words[1]);
					auto column = parseNumber<Index>(words[2]);
					player->spawnPosition = {row, column};
				} catch (const std::invalid_argument &) {
					return {false, "Invalid parameters."};
				}

				return {true, ""};
			}

			if (first == "boom") {
				float radius = 2;

				if (words.size() > 1) {
					try {
						radius = parseNumber<float>(words[1]);
					} catch (const std::invalid_argument &) {
						return {false, "Couldn't parse radius."};
					}
				}

				causeExplosion(player->getPlace(), radius, false);
				return {true, ""};
			}

		} catch (const std::exception &err) {
			return {false, err.what()};
		}

		return {false, "Unknown command."};
	}

	void ServerGame::broadcast(const Place &place, const PacketPtr &packet) {
		auto lock = players.sharedLock();
		for (const ServerPlayerPtr &player: players) {
			if (player->canSee(place.realm->id, place.position)) {
				if (auto client = player->toServer()->weakClient.lock()) {
					client->send(packet);
				}
			}
		}
	}

	Token ServerGame::generateRandomToken() {
		std::random_device rng;
		return (Token(rng()) << 32) | rng();
	}
}
