#include "Log.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/RemoteClient.h"
#include "packet/InventorySlotUpdatePacket.h"

namespace Game3 {
	void InventorySlotUpdatePacket::handle(ClientGame &game) {
		if (auto player = game.player; player && player->inventory) {
			if (auto stack_pointer = (*player->inventory)[slot])
				*stack_pointer = stack;
			else
				player->inventory->add(stack, slot);
		}
	}
}
