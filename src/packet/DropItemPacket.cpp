#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/DropItemPacket.h"

namespace Game3 {
	void DropItemPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		const InventoryPtr inventory = client.getPlayer()->getInventory(0);

		if (!inventory) {
			client.send(make<ErrorPacket>("Can't drop/discard item: no inventory"));
			return;
		}

		auto inventory_lock = inventory->uniqueLock();

		if (!inventory->hasSlot(slot)) {
			client.send(make<ErrorPacket>("Can't drop/discard item: invalid slot"));
			return;
		}

		if (discard)
			inventory->erase(slot);
		else
			inventory->drop(slot);

		inventory->notifyOwner({});
	}
}
