#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/RemoteClient.h"
#include "packet/InventorySlotUpdatePacket.h"

namespace Game3 {
	void InventorySlotUpdatePacket::handle(ClientGame &game) {
		if (auto player = game.player) {
			if (const InventoryPtr inventory = player->getInventory()) {
				if (auto stack_pointer = (*inventory)[slot])
					*stack_pointer = stack;
				else
					inventory->add(stack, slot);
			}
		}
	}
}
