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
#include "packet/InventoryPacket.h"
#include "packet/TileEntityPacket.h"
#include "packet/EntityMovedPacket.h"
#include "packet/TileUpdatePacket.h"
#include "packet/TimePacket.h"
#include "util/Util.h"

namespace Game3 {
	void ServerGame::addEntityFactories() {
		Game::addEntityFactories();
		add(EntityFactory::create<ServerPlayer>());
	}

	void ServerGame::tick() {
		Game::tick();

		std::vector<RemoteClient::BufferGuard> guards;
		guards.reserve(players.size());

		for (const auto &player: players)
			if (auto client = player->toServer()->weakClient.lock())
				guards.emplace_back(client);

		for (const auto &[client, packet]: packetQueue.steal())
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
				player->send(InventoryPacket(player->inventory));
				player->inventoryUpdated = false;
			}
		}


		for (const auto &player: playerRemovalQueue.steal()) {
			remove(player);
			player->toServer()->weakClient.reset();
			if (auto count = player.use_count(); count != 1)
				WARN("Player ref count: " << count << " (should be 1)");
		}

		lastGarbageCollection += delta;
		if (GARBAGE_COLLECTION_TIME <= lastGarbageCollection) {
			garbageCollect();
			lastGarbageCollection = 0.f;
		}
	}

	void ServerGame::garbageCollect() {
		std::shared_lock lock(playersMutex);
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
		const TileUpdatePacket packet(realm_id, layer, position, tile_id);
		auto lock = lockPlayersShared();
		for (const auto &player: players)
			if (player->canSee(realm_id, position))
				if (auto client = player->toServer()->weakClient.lock())
					client->send(packet);
	}

	void ServerGame::queuePacket(std::shared_ptr<RemoteClient> client, std::shared_ptr<Packet> packet) {
		packetQueue.emplace(std::move(client), std::move(packet));
	}

	void ServerGame::runCommand(RemoteClient &client, const std::string &command, GlobalID command_id) {
		auto [success, message] = commandHelper(client, command);
		CommandResultPacket packet(command_id, success, std::move(message));
		client.send(packet);
	}

	void ServerGame::entityTeleported(Entity &entity) {
		if (entity.spawning)
			return;

		const EntityMovedPacket packet(entity);

		if (auto cast_player = dynamic_cast<Player *>(&entity)) {
			cast_player->send(packet);
			auto lock = lockPlayersShared();
			for (const auto &player: players)
				if (player.get() != cast_player && player->getRealm() && player->canSee(entity))
					if (auto client = player->toServer()->weakClient.lock())
						client->send(packet);
		} else {
			auto lock = lockPlayersShared();
			for (const auto &player: players)
				if (player->getRealm() && player->canSee(entity))
					if (auto client = player->toServer()->weakClient.lock())
						client->send(packet);
		}
	}

	void ServerGame::entityDestroyed(const Entity &entity) {
		const DestroyEntityPacket packet(entity);
		auto lock = server->server->lockClients();
		for (const auto &[client_id, client]: server->server->getClients())
			std::dynamic_pointer_cast<RemoteClient>(client)->send(packet);
	}

	void ServerGame::tileEntitySpawned(const TileEntityPtr &tile_entity) {
		const TileEntityPacket packet(tile_entity);
		auto realm = tile_entity->getRealm();
		ChunkRange(tile_entity->getChunk()).iterate([&](ChunkPosition chunk_position) {
			if (auto entities = realm->getEntities(chunk_position)) {
				auto lock = entities->sharedLock();
				for (const auto &entity: *entities)
					if (entity->isPlayer())
						entity->cast<ServerPlayer>()->send(packet);
			}
		});
	}

	void ServerGame::tileEntityDestroyed(const TileEntity &tile_entity) {
		const DestroyTileEntityPacket packet(tile_entity);
		auto lock = server->server->lockClients();
		for (const auto &[client_id, client]: server->server->getClients())
			std::dynamic_pointer_cast<RemoteClient>(client)->send(packet);
	}

	void ServerGame::remove(const ServerPlayerPtr &player) {
		auto lock = lockPlayersUnique();
		players.erase(player);
		player->destroy();
	}

	void ServerGame::queueRemoval(const ServerPlayerPtr &player) {
		playerRemovalQueue.push(player);
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
				if (3 < words.size())
					return {false, "Too many arguments."};
				long count = 1;
				if (words.size() == 3) {
					try {
						count = parseLong(words.at(2));
					} catch (const std::invalid_argument &) {
						return {false, "Invalid count."};
					}
				}
				auto item_name = std::string(words.at(1));
				size_t colon = item_name.find(':');
				if (colon == item_name.npos)
					item_name = "base:item/" + std::string(item_name);
				if (auto item = registry<ItemRegistry>()[Identifier(item_name)]) {
					player->give(ItemStack(*this, item, count));
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
			}
		} catch (const std::exception &err) {
			return {false, err.what()};
		}

		return {false, "Unknown command."};
	}
}
