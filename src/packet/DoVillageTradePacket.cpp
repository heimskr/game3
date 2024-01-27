#include "algorithm/Stonks.h"
#include "entity/ServerPlayer.h"
#include "error/InsufficientFundsError.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/DoVillageTradePacket.h"

namespace Game3 {
	void DoVillageTradePacket::handle(ServerGame &game, RemoteClient &client) {
		// Validate player

		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			client.sendError("No player.");
			return;
		}

		// Validate item

		ItemPtr item = game.getItem(resource);
		if (!item) {
			client.sendError("Resource {} not found.", resource);
			return;
		}

		// Lock inventory

		InventoryPtr inventory = player->getInventory(0);
		auto inventory_lock = inventory->uniqueLock();

		// Validate village

		VillagePtr village;

		try {
			village = game.getVillage(villageID);
		} catch (const std::out_of_range &) {
			client.sendError("Village {} not found.", villageID);
			return;
		}

		// Find resource count

		ItemCount resource_count{};
		auto resources_lock = village->getResources().uniqueLock();

		if (auto found_count = village->getResourceAmount(resource)) {
			resource_count = *found_count;
		} else {
			client.sendError("Resource not found in village {}: {}", *village, resource);
			return;
		}

		// Handle selling

		if (isSell) {
			if (ItemCount contained = inventory->count(resource); contained < amount) {
				client.sendError("Can't sell {} x {}: player has only {}", amount, resource, contained);
				return;
			}

			if (std::optional<MoneyCount> sell_price = totalSellPrice(resource_count, -1, item->basePrice, amount)) {
				player->addMoney(*sell_price);
				inventory->remove(ItemStack(game, resource, amount));
				village->setResourceAmount(resource, resource_count + amount);
				return;
			}

			client.sendError("Can't sell {} x {}: village doesn't have enough money", amount, resource);
			return;
		}

		// Handle buying

		if (std::optional<MoneyCount> buy_price = totalBuyPrice(resource_count, -1, item->basePrice, amount)) {
			if (!player->removeMoney(*buy_price)) {
				client.sendError("Village trade failed: insufficient funds (have {}, need {})", player->getMoney(), *buy_price);
				return;
			}

			village->setResourceAmount(resource, resource_count - amount);

			if (auto leftover = inventory->add(ItemStack(game, resource, amount)))
				leftover->spawn(player->getRealm(), player->getPosition());

			return;
		}

		client.sendError("Can't buy {} x {}: village doesn't have enough", amount, resource);
	}
}
