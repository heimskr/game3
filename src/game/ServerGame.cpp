#include "ThreadContext.h"
#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/CommandResultPacket.h"
#include "packet/TileUpdatePacket.h"
#include "util/Util.h"

namespace Game3 {
	void ServerGame::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1'000'000'000.;
		for (auto &[id, realm]: realms)
			realm->tick(delta);
		for (const auto &player: players)
			player->ticked = false;
	}

	void ServerGame::broadcastTileUpdate(RealmID realm_id, Layer layer, const Position &position, TileID tile_id) {
		auto lock = lockPlayersShared();
		for (const auto &player: players) {
			if (player->canSee(realm_id, position)) {
				TileUpdatePacket packet(realm_id, layer, position, tile_id);
				player->client->send(packet);
			}
		}
	}

	void ServerGame::runCommand(const PlayerPtr &player, const std::string &command, GlobalID command_id) {
		auto [success, message] = commandHelper(player, command);
		CommandResultPacket packet(command_id, success, std::move(message));
		player->client->send(packet);
	}

	void ServerGame::handlePacket(RemoteClient &client, const Packet &packet) {
		packet.handle(*this, client);
	}

	std::tuple<bool, std::string> ServerGame::commandHelper(const PlayerPtr &player, const std::string &command) {
		if (command.empty())
			return {false, "Command is empty."};

		const auto words = split(command, " ", false);
		const auto &first = words.at(0);

		// TODO: command registry...?

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
				std::string_view item_name = words.at(1);
				size_t colon = item_name.find(':');
				if (colon == item_name.npos)
					item_name = "base:item/" + std::string(item_name);
				if (auto item = registry<ItemRegistry>()[Identifier(item_name)]) {
					player->give(ItemStack(*this, item, count));
					return {true, "Gave " + std::to_string(count) + " x " + item->name};
				}
				return {false, "Unknown item: " + std::string(item_name)};
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
				runCommand(player, "heldL 0", threadContext.rng());
				runCommand(player, "heldR 1", threadContext.rng());
			}
		} catch (const std::exception &err) {
			return {false, err.what()};
		}

		return {false, "Unknown command."};
	}
}
