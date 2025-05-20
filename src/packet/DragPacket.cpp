#include "util/Log.h"
#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/DragPacket.h"

namespace Game3 {
	void DragPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		PlayerPtr player = client.getPlayer();
		if (!player) {
			return;
		}

		const InventoryPtr inventory = player->getInventory(0);

		ItemStackPtr active_stack;
		Slot active_slot{};
		{
			auto lock = inventory->sharedLock();
			active_stack = inventory->getActive();
			active_slot = inventory->activeSlot;
		}

		if (active_stack) {
			active_stack->item->drag(active_slot, active_stack, {position, player->getRealm(), player}, modifiers, offsets, action);
		}
	}
}
