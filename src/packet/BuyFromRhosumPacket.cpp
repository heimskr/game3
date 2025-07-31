#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/Item.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/BuyFromRhosumPacket.h"
#include "packet/PurchaseResultPacket.h"

namespace Game3 {
	void BuyFromRhosumPacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			return;
		}

		constexpr std::array choices{
			"base:item/lamp_oil",
			"base:item/rope",
			"base:item/bomb",
		};

		if (index < 0 || choices.size() <= static_cast<size_t>(index)) {
			player->send(make<ErrorPacket>(std::format("Invalid index: {}", index)));
			return;
		}

		ItemStackPtr stack = ItemStack::create(game, choices[index], 1);
		InventoryPtr inventory = player->getInventory(0);

		if (!player->removeMoney(stack->item->basePrice)) {
			player->send(make<PurchaseResultPacket>(false));
			return;
		}

		if (inventory->add(stack)) {
			player->addMoney(stack->item->basePrice);
			player->send(make<PurchaseResultPacket>(false));
			return;
		}

		inventory->notifyOwner(stack);
		player->send(make<PurchaseResultPacket>(true));
	}
}
