#include "Log.h"
#include "threading/ThreadContext.h"
#include "entity/EntityFactory.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
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
#include "util/Util.h"

namespace Game3 {
	void ServerGame::addEntityFactories() {
		Game::addEntityFactories();
		add(EntityFactory::create<ServerPlayer>());
	}

	ServerGame::~ServerGame() {
		INFO("Saving realms and users...");
		database.writeAllRealms();
		database.writeUsers(players);
		SUCCESS("Saved realms and users.");
	}

	bool ServerGame::tick() {
		if (!Game::tick())
			return false;

		std::vector<RemoteClient::BufferGuard> guards;
		guards.reserve(players.size());

		for (const auto &player: players)
			if (auto client = player->toServer()->weakClient.lock())
				guards.emplace_back(client);

		for (const auto &[weak_client, packet]: packetQueue.steal())
			if (auto client = weak_client.lock())
				handlePacket(*client, *packet);

		for (auto &[id, realm]: realms)
			realm->tick(delta);

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
				remove(player);
				player->toServer()->weakClient.reset();
				player->clearQueues();
				if (auto count = player.use_count(); count != 1)
					WARN("Player " << player.get() << " ref count: " << count << " (should be 1)");

				for (const auto &[id, realm]: realms) {
					{
						auto lock = realm->entities.sharedLock();
						if (realm->entities.contains(player))
							INFO("Still present in Realm " << id << "'s entities");
					}

					{
						auto lock = realm->entitiesByGID.sharedLock();
						if (realm->entitiesByGID.contains(player->getGID()))
							INFO("Still present in Realm " << id << "'s entitiesByGID");
					}

					{
						auto lock = realm->entitiesByChunk.sharedLock();
						for (const auto &[cpos, set]: realm->entitiesByChunk) {
							if (set) {
								auto set_lock = set->sharedLock();
								if (set->contains(player))
									INFO("Still present in Realm " << id << "'s entitiesByChunk at chunk position " << cpos);
							}
						}
					}
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
		for (const auto &player: players)
			if (player->knowsRealm(new_realm->id))
				player->send(moved_packet);
			else
				player->send(changing_packet);
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
		auto lock = server->server->lockClients();
		for (const auto &[client_id, client]: server->server->getClients())
			std::static_pointer_cast<RemoteClient>(client)->send(packet);
	}

	void ServerGame::tileEntitySpawned(const TileEntityPtr &tile_entity) {
		const TileEntityPacket packet(tile_entity);
		auto realm = tile_entity->getRealm();
		realm->updateNeighbors(tile_entity->getPosition());
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
		auto lock = server->server->lockClients();
		for (const auto &[client_id, client]: server->server->getClients())
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
			} else if (first == "heldL" || first == "heldR") {
				if (words.size() != 2)
					return {false, "Invalid number of arguments."};
				long slot = -1;
				try {
					slot = parseLong(words.at(1));
				} catch (const std::invalid_argument &) {
					return {false, "Invalid slot."};
				}
				if (first == "heldL")
					player->setHeldLeft(slot);
				else
					player->setHeldRight(slot);
				return {true, "Set held slot to " + std::to_string(slot)};
			} else if (first == "h") {
				runCommand(client, "heldL 0", threadContext.rng());
				runCommand(client, "heldR 1", threadContext.rng());
			} else if (first == "go") {
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
			} else if (first == "counter") {
				const auto chunk = player->getChunk();
				const auto counter = player->getRealm()->tileProvider.getUpdateCounter(chunk);
				return {true, "Counter for chunk " + static_cast<std::string>(chunk) + ": " + std::to_string(counter)};
			} else if (first == "moving") {
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
			} else if (first == "submerge") {
				if (words.size() != 2)
					return {false, "Invalid number of arguments."};
				player->getRealm()->setTile(Layer::Submerged, player->getPosition(), words.at(1));
				return {true, "Set tile."};
			} else if (first == "saveall") {
				INFO("Writing...");
				database.writeAllRealms();
				INFO("Writing done.");
				return {true, "Wrote all chunks."};
			} else if (first == "pos") {
				INFO("Player " << player->getGID() << " position: " << player->getPosition());
				INFO("Player " << player->getGID() << " chunk position: " << player->getChunk());
				return {true, "Position = " + std::string(player->getPosition()) + ", chunk position = " + std::string(player->getChunk())};
			} else if (first == "pm") {
				if (1 < words.size())
					player->getRealm()->remakePathMap(player->getChunk());

				TileProvider &provider = player->getRealm()->tileProvider;
				auto &path_chunk = provider.getPathChunk(player->getChunk());
				auto lock = path_chunk.sharedLock();
				size_t walkables = 0;
				for (size_t y = 0; y < CHUNK_SIZE; ++y) {
					for (size_t x = 0; x < CHUNK_SIZE; ++x) {
						const auto walkable = path_chunk[y * CHUNK_SIZE + x];
						std::cerr << int(walkable);
						walkables += walkable;
					}
					std::cerr << '\n';
				}
				return {true, "Walkable: " + std::to_string(walkables) + " / " + std::to_string(CHUNK_SIZE * CHUNK_SIZE)};
			} else if (first == "stop") {
				auto server = weakServer.lock();
				if (!server)
					return {false, "Couldn't lock server."};
				server->stop();
				return {true, "Stopped server."};
			}
		} catch (const std::exception &err) {
			return {false, err.what()};
		}

		return {false, "Unknown command."};
	}
}
