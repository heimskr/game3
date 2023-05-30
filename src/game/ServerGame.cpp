#include "Log.h"
#include "threading/ThreadContext.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/CommandResultPacket.h"
#include "packet/DestroyEntityPacket.h"
#include "packet/DestroyTileEntityPacket.h"
#include "packet/EntityMovedPacket.h"
#include "packet/TileUpdatePacket.h"
#include "util/Util.h"

namespace Game3 {
	void ServerGame::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1'000'000'000.;

		std::vector<RemoteClient::BufferGuard> guards;
		guards.reserve(players.size());

		for (const auto &player: players)
			if (auto client = player->client.lock())
				guards.emplace_back(client);

		for (const auto &[client, packet]: packetQueue.steal())
			handlePacket(*client, *packet);

		for (auto &[id, realm]: realms)
			realm->tick(delta);

		for (const auto &player: players)
			player->ticked = false;

		for (auto player: playerRemovalQueue.steal()) {
			remove(player);
			player->client.reset();
		}
	}

	void ServerGame::broadcastTileUpdate(RealmID realm_id, Layer layer, const Position &position, TileID tile_id) {
		const TileUpdatePacket packet(realm_id, layer, position, tile_id);
		auto lock = lockPlayersShared();
		for (const auto &player: players)
			if (player->canSee(realm_id, position))
				if (auto client = player->client.lock())
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
					if (auto client = player->client.lock())
						client->send(packet);
		} else {
			auto lock = lockPlayersShared();
			for (const auto &player: players)
				if (player->getRealm() && player->canSee(entity))
					if (auto client = player->client.lock())
						client->send(packet);
		}
	}

	void ServerGame::entityDestroyed(const Entity &entity) {
		const DestroyEntityPacket packet(entity);
		auto lock = server->server->lockClients();
		for (const auto &[client_id, client]: server->server->getClients())
			std::dynamic_pointer_cast<RemoteClient>(client)->send(packet);
	}

	void ServerGame::tileEntityDestroyed(const TileEntity &tile_entity) {
		const DestroyTileEntityPacket packet(tile_entity);
		auto lock = server->server->lockClients();
		for (const auto &[client_id, client]: server->server->getClients())
			std::dynamic_pointer_cast<RemoteClient>(client)->send(packet);
	}

	void ServerGame::remove(const PlayerPtr &player) {
		auto lock = lockPlayersUnique();
		players.erase(player);
		player->destroy();
	}

	void ServerGame::queueRemoval(const PlayerPtr &player) {
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
