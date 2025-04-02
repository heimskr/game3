#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/RemoteClient.h"
#include "packet/InventorySlotUpdatePacket.h"

namespace Game3 {
	void InventorySlotUpdatePacket::handle(const ClientGamePtr &game) {
		if (ClientPlayerPtr player = game->getPlayer()) {
			if (InventoryPtr inventory = player->getInventory(0)) {
				auto inventory_lock = inventory->uniqueLock();
				if (ItemStackPtr stack_pointer = (*inventory)[slot]) {
					*stack_pointer = *stack;
				} else {
					inventory->add(stack, slot);
				}
			}
		}
	}
}
