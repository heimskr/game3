#include "Log.h"
#include "threading/ThreadContext.h"
#include "entity/Animal.h"
#include "entity/EntityFactory.h"
#include "entity/ItemEntity.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
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
#include "util/Demangle.h"
#include "util/Timer.h"
#include "util/Util.h"

#include <iomanip>

namespace Game3 {
	ServerGame::ServerGame(const std::shared_ptr<Server> &server_, size_t pool_size):
		weakServer(server_), pool(pool_size) { pool.start(); }

	void ServerGame::addEntityFactories() {
		Game::addEntityFactories();
		add(EntityFactory::create<ServerPlayer>());
	}

	ServerGame::~ServerGame() {
		pool.join();
		INFO("Saving realms and users...");
		database.writeAllRealms();
		database.writeUsers(players);
		SUCCESS("Saved realms and users.");
		Timer::summary();
		Timer::clear();
	}

	bool ServerGame::tick() {
		if (!Game::tick())
			return false;

		std::unordered_map<Player *, RemoteClient::BufferGuard> guards;
		guards.reserve(players.size());

		for (const auto &player: players)
			if (auto client = player->toServer()->weakClient.lock())
				guards.emplace(player.get(), client);

		for (const auto &[weak_client, packet]: packetQueue.steal())
			if (auto client = weak_client.lock())
				handlePacket(*client, *packet);

		const size_t max_jobs = realms.size() * 2;

		for (auto &[id, realm]: realms) {
			if (max_jobs <= pool.jobCount())
				break;
			// pool.add([weak_realm = std::weak_ptr(realm), delta = delta](ThreadPool &, size_t) {
			// 	if (RealmPtr realm = weak_realm.lock())
					realm->tick(delta);
			// });
		}

		std::optional<TimePacket> time_packet;
		timeSinceTimeUpdate += delta;
		if (10. <= timeSinceTimeUpdate) {
			time_packet.emplace(time);
			timeSinceTimeUpdate = 0.;
		}

		for (const auto &player: players) {
			player->ticked = false;

			if (time_packet)
				player->send(*time_packet);

			if (player->inventoryUpdated) {
				player->send(InventoryPacket(player->getInventory()));
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
					{
						auto lock = realm->entities.uniqueLock();
						if (auto iter = realm->entities.find(player); iter != realm->entities.end()) {
							WARN("Still present in Realm " << id << "'s entities");
							realm->entities.erase(iter);
						}
					}

					{
						auto &by_gid = realm->entitiesByGID;
						auto lock = by_gid.uniqueLock();
						if (auto iter = by_gid.find(player->getGID()); iter != by_gid.end()) {
							WARN("Still present in Realm " << id << "'s entitiesByGID");
							by_gid.erase(iter);
						}
					}

					{
						auto lock = realm->entitiesByChunk.sharedLock();
						for (const auto &[chunk_position, set]: realm->entitiesByChunk) {
							if (!set)
								continue;
							auto set_lock = set->uniqueLock();
							if (auto iter = set->find(player); iter != set->end()) {
								WARN("Still present in Realm " << id << "'s entitiesByChunk at chunk position " << chunk_position);
								set->erase(iter);
							}
						}
					}
				}

				if (auto count = player.use_count(); count != 1) {
					WARN("Player " << player.get() << " ref count: " << count << " (should be 1). Current realm: "
						<< player->realmID << " (realmID) or " << player->getRealm()->id << " (getRealm()->id)");
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
				if (player->knownEntities.empty())
					continue;
			}
			auto unique_lock = player->knownEntities.uniqueLock();
			std::vector<std::weak_ptr<Entity>> to_remove;
			to_remove.reserve(player->knownEntities.size() / 4);
			for (const auto &weak_entity: player->knownEntities)
				if (!weak_entity.lock())
					to_remove.push_back(weak_entity);
			for (const auto &weak_entity: to_remove)
				player->knownEntities.erase(weak_entity);
		}
	}

	void ServerGame::broadcastTileUpdate(RealmID realm_id, Layer layer, const Position &position, TileID tile_id) {
		broadcast({position, realms.at(realm_id), nullptr}, TileUpdatePacket(realm_id, layer, position, tile_id));
	}

	void ServerGame::broadcastFluidUpdate(RealmID realm_id, const Position &position, FluidTile tile) {
		broadcast({position, realms.at(realm_id), nullptr}, FluidUpdatePacket(realm_id, position, tile));
	}

	void ServerGame::queuePacket(std::shared_ptr<RemoteClient> client, std::shared_ptr<Packet> packet) {
		packetQueue.emplace(std::move(client), std::move(packet));
	}

	void ServerGame::runCommand(RemoteClient &client, const std::string &command, GlobalID command_id) {
		auto [success, message] = commandHelper(client, command);
		client.send(CommandResultPacket(command_id, success, std::move(message)));
	}

	void ServerGame::entityChangingRealms(Entity &entity, const RealmPtr &new_realm, const Position &new_position) {
		const EntityChangingRealmsPacket changing_packet(entity.getGID(), new_realm->id, new_position);
		EntityMovedPacket moved_packet(entity);
		moved_packet.arguments.position = new_position;
		moved_packet.arguments.isTeleport = true;

		auto lock = players.sharedLock();
		for (const auto &player: players) {
			if (player->knowsRealm(new_realm->id))
				player->send(moved_packet);
			else
				player->send(changing_packet);
		}
	}

	void ServerGame::entityTeleported(Entity &entity, MovementContext context) {
		if (entity.spawning)
			return;

		EntityMovedPacket packet(entity);
		packet.arguments.isTeleport = context.isTeleport;

		// Actual teleportation (rather than regular movement between adjacent tiles) should be instant.
		if (context.isTeleport)
			packet.arguments.adjustOffset = false;

		if (auto cast_player = dynamic_cast<Player *>(&entity)) {
			if (!context.excludePlayerSelf)
				cast_player->send(packet);
			auto lock = players.sharedLock();
			for (const auto &player: players)
				if (player.get() != cast_player && player->getRealm() && player->canSee(entity))
					if (auto client = player->toServer()->weakClient.lock())
						client->send(packet);
		} else {
			auto lock = players.sharedLock();
			for (const auto &player: players)
				if (player->getRealm() && player->canSee(entity))
					if (auto client = player->toServer()->weakClient.lock())
						client->send(packet);
		}
	}

	void ServerGame::entityDestroyed(const Entity &entity) {
		const DestroyEntityPacket packet(entity, false);
		auto server = weakServer.lock();
		assert(server);
		auto &clients = server->getClients();
		auto lock = clients.sharedLock();
		for (const auto &client: clients)
			std::static_pointer_cast<RemoteClient>(client)->send(packet);
	}

	void ServerGame::tileEntitySpawned(const TileEntityPtr &tile_entity) {
		const TileEntityPacket packet(tile_entity);
		auto realm = tile_entity->getRealm();
		realm->updateNeighbors(tile_entity->getPosition(), Layer::Submerged);
		realm->updateNeighbors(tile_entity->getPosition(), Layer::Objects);
		ChunkRange(tile_entity->getChunk()).iterate([&](ChunkPosition chunk_position) {
			if (auto entities = realm->getEntities(chunk_position)) {
				auto lock = entities->sharedLock();
				for (const auto &entity: *entities)
					if (entity->isPlayer())
						std::static_pointer_cast<ServerPlayer>(entity)->send(packet);
			}
		});
	}

	void ServerGame::tileEntityDestroyed(const TileEntity &tile_entity) {
		const DestroyTileEntityPacket packet(tile_entity);
		auto server = weakServer.lock();
		assert(server);
		auto &clients = server->getClients();
		auto lock = clients.sharedLock();
		for (const auto &client: clients)
			std::dynamic_pointer_cast<RemoteClient>(client)->send(packet);
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
		database.open(std::move(path));
	}

	void ServerGame::broadcast(const Packet &packet) {
		auto lock = players.sharedLock();
		for (const ServerPlayerPtr &player: players)
			player->send(packet);
	}

	void ServerGame::releasePlayer(const std::string &username, const Place &place) {
		auto lock = playerMap.sharedLock();
		if (auto iter = playerMap.find(username); iter != playerMap.end()) {
			iter->second->teleport(place.position, place.realm, MovementContext{.isTeleport = true});
		} else {
			lock.unlock();
			database.writeReleasePlace(username, place);
		}
	}

	void ServerGame::handlePacket(RemoteClient &client, Packet &packet) {
		packet.handle(*this, client);
	}

	std::tuple<bool, std::string> ServerGame::commandHelper(RemoteClient &client, const std::string &command) {
		if (command.empty())
			return {false, "Command is empty."};

		const auto words = split(command, " ", false);
		const auto &first = words.at(0);
		auto player = client.getPlayer();

		// Change this check if there are any miscellaneous commands implemented later that don't require a player.
		if (!player)
			return {false, "No player."};

		try {
			if (first == "give") {
				if (words.size() < 2)
					return {false, "Not enough arguments."};

				long count = 1;
				if (3 <= words.size()) {
					try {
						count = parseLong(words.at(2));
					} catch (const std::invalid_argument &) {
						return {false, "Invalid count."};
					}
				}

				nlohmann::json data;
				if (3 < words.size()) {
					try {
						data = nlohmann::json::parse(join(std::span(words.begin() + 3, words.end()), " "));
					} catch (const std::exception &err) {
						ERROR(err.what());
						return {false, "Couldn't parse data as JSON."};
					}
				}

				std::string item_name(words.at(1));
				const size_t colon = item_name.find(':');

				if (colon == item_name.npos)
					item_name = "base:item/" + std::string(item_name);

				if (auto item = registry<ItemRegistry>()[Identifier(item_name)]) {
					player->give(ItemStack(*this, item, count, std::move(data)));
					return {true, "Gave " + std::to_string(count) + " x " + item->name};
				}

				return {false, "Unknown item: " + item_name};
			}

			if (first == "heldL" || first == "heldR") {
				if (words.size() != 2)
					return {false, "Invalid number of arguments."};
				Slot slot = -1;
				try {
					slot = parseNumber<Slot>(words.at(1));
				} catch (const std::invalid_argument &) {
					return {false, "Invalid slot."};
				}
				if (first == "heldL")
					player->setHeldLeft(slot);
				else
					player->setHeldRight(slot);
				return {true, "Set held slot to " + std::to_string(slot)};
			}

			if (first == "h") {
				runCommand(client, "heldL 0", threadContext.rng());
				runCommand(client, "heldR 1", threadContext.rng());
			}

			if (first == "go") {
				if (words.size() != 3)
					return {false, "Invalid number of arguments."};

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
						if (first)
							first = false;
						else
							ss << ", ";
						ss << move;
					}
					ss << ". Offset: " << player->offset.x << ", " << player->offset.y << ", " << player->offset.z;
				} else {
					ss << "Player isn't moving. Offset: " << player->offset.x << ", " << player->offset.y << ", " << player->offset.z;
				}
				return {true, ss.str()};
			}

			if (first == "submerge" || first == "terrain") {
				if (words.size() != 2)
					return {false, "Invalid number of arguments."};
				player->getRealm()->setTile(first == "submerge"? Layer::Submerged : Layer::Terrain, player->getPosition(), words.at(1));
				return {true, "Set tile."};
			}

			if (first == "saveall") {
				INFO("Writing...");
				database.writeAllRealms();
				INFO("Writing done.");
				return {true, "Wrote all chunks."};
			}

			if (first == "pos") {
				INFO("Player " << player->getGID() << " position: " << player->getPosition());
				INFO("Player " << player->getGID() << " chunk position: " << player->getChunk());
				return {true, "Position = " + std::string(player->getPosition()) + ", chunk position = " + std::string(player->getChunk())};
			}

			if (first == "pm") {
				if (1 < words.size())
					player->getRealm()->remakePathMap(player->getChunk());

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
				if (player->username != "heimskr")
					return {false, "No thanks."};
				auto server = weakServer.lock();
				if (!server)
					return {false, "Couldn't lock server."};
				server->stop();
				return {true, "Stopped server."};
			}

			if (first == "say" || first == ":") {
				std::string_view message = std::string_view(command).substr(first.size() + 1);
				INFO('[' << player->username << "] " << message);
				broadcast(ChatMessageSentPacket{player->getGID(), std::string(message)});
				return {true, ""};
			}

			if (first == "goto") {
				auto server = weakServer.lock();
				if (!server)
					return {false, "Couldn't lock server."};

				if (words.size() != 2)
					return {false, "Invalid number of arguments."};

				ServerPlayerPtr other_player;
				{
					const std::string other_username(words.at(1));
					auto lock = playerMap.sharedLock();
					if (auto iter = playerMap.find(other_username); iter != playerMap.end())
						other_player = iter->second;
					else
						return {false, "Couldn't find player " + other_username + "."};
				}

				player->teleport(other_player->getPosition(), other_player->getRealm(), {
					.isTeleport = true
				});

				return {true, "Teleported."};
			}

			if (first == "realm") {
				if (words.size() != 2)
					return {false, "Invalid number of arguments."};

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

				player->teleport(player->getPosition(), realm, {
					.isTeleport = true
				});

				return {true, "Teleported."};
			}

			if (first == "texture") {
				if (words.size() != 1 && words.size() != 2)
					return {false, "Invalid number of arguments."};

				Identifier choice;

				if (words.size() < 2)
					choice = "base:entity/player";
				else if (words[1].find(':') != std::string_view::npos)
					choice = words[1];
				else
					choice = Identifier("base:entity/" + std::string(words[1]));

				auto &textures = registry<EntityTextureRegistry>();
				if (!textures.contains(choice))
					return {false, "Invalid entity texture."};

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
				{
					auto lock = players.sharedLock();
					for (const auto &iterated_player: players)
						display_names.insert(iterated_player->displayName);
				}
				return {true, "Online players: " + join(display_names, ", ")};
			}

			if (first == "entities" || first == "ents") {
				auto lock = allAgents.sharedLock();
				std::vector<EntityPtr> entities;

				auto has_arg = [&](const char *arg) {
					for (const std::string_view &word: words)
						if (word == arg)
							return true;
					return false;
				};

				const bool exclude_both    = has_arg("-ai");
				const bool exclude_animals = exclude_both || has_arg("-a");
				const bool exclude_items   = exclude_both || has_arg("-i");

				for (const auto &[gid, weak_agent]: allAgents) {
					if (AgentPtr agent = weak_agent.lock()) {
						if (GlobalID agent_gid = agent->getGID(); gid != agent_gid)
							WARN("Agent " << agent_gid << " is stored in allAgents with key " << gid);
						if (auto entity = std::dynamic_pointer_cast<Entity>(agent))
							if (!exclude_animals || !std::dynamic_pointer_cast<Animal>(entity))
								if (!exclude_items || !std::dynamic_pointer_cast<ItemEntity>(entity))
									entities.push_back(entity);
					}
				}

				std::sort(entities.begin(), entities.end(), [](const EntityPtr &left, const EntityPtr &right) {
					const std::string left_demangled  = DEMANGLE(*left);
					const std::string right_demangled = DEMANGLE(*right);
					if (left_demangled < right_demangled)
						return true;
					if (left_demangled > right_demangled)
						return false;


					const std::string left_name  = left->getName();
					const std::string right_name = right->getName();
					if (left_name < right_name)
						return true;
					if (left_name > right_name)
						return false;

					return left->getGID() < right->getGID();
				});

				for (const EntityPtr &entity: entities) {
					Entity &entity_ref = *entity;
					INFO((entity->isPlayer()? "\e[1m(" + std::dynamic_pointer_cast<Player>(entity)->username + ") \e[22;2m" : "\e[2m") << std::setw(20) << std::right
						<< entity->getGID() << "\e[22m  " << DEMANGLE(entity_ref) << "  \e[32m" << entity->getName() << "\e[39m  Realm \e[31m" << entity->getRealm()->id
						<< "\e[39m  Position \e[33m" << entity->getPosition() << "\e[39m");
				}

				return {true, ""};
			}

			if (first == "tiles") {
				RealmPtr realm = player->getRealm();
				Tileset &tileset = realm->getTileset();
				for (const Layer layer: allLayers)
					if (auto tile = realm->tryTile(layer, player->position))
						INFO(getIndex(layer) << " \e[2m→\e[22m " << *tile << " \e[2m/\e[22m " << tileset[*tile]);
				return {true, ""};
			}

			if (first == "regen") {
				if (player->username != "heimskr")
					return {false, "No thanks."};
				if (words.size() != 3)
					return {false, "Incorrect parameter count."};

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

		} catch (const std::exception &err) {
			return {false, err.what()};
		}

		return {false, "Unknown command."};
	}
}
