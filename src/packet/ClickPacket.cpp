#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/GenericClient.h"
#include "packet/ClickPacket.h"
#include "util/Log.h"

namespace Game3 {
	void ClickPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			return;
		}

		InventoryPtr inventory = player->getInventory(0);

		if (ItemStackPtr stack = inventory->getActive()) {
			stack->item->use(inventory->activeSlot, stack, Place{position, player->getRealm(), player}, modifiers, {offsetX, offsetY});
		}
	}
}
