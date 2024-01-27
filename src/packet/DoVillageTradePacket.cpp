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

		// Handle selling

		if (isSell) {
			if (ItemCount contained = inventory->count(resource); contained < amount) {
				client.sendError("Can't sell {} x {}: player has only {}", amount, resource, contained);
				return;
			}

			inventory->remove(ItemStack(game, resource, amount));
			const MoneyCount sell_price(std::floor(sellPrice(item->basePrice, amount, -1, 0.0)));
			player->addMoney(sell_price);
			return;
		}

		// Handle buying

		const MoneyCount buy_price(std::ceil(buyPrice(item->basePrice, amount, -1)));

		if (!player->removeMoney(buy_price)) {
			client.sendError("Village trade failed: insufficient funds (have {}, need {})", player->getMoney(), buy_price);
			return;
		}

		INFO("Bought {} for {} from {}", ItemStack(game, resource, amount), buy_price, *village);

		if (auto leftover = inventory->add(ItemStack(game, resource, amount)))
			leftover->spawn(player->getRealm(), player->getPosition());
	}
}
