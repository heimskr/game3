#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/DropItemPacket.h"

namespace Game3 {
	void DropItemPacket::handle(ServerGame &, RemoteClient &client) {
		auto player = client.getPlayer();
		auto inventory = player->inventory;

		if (!inventory) {
			client.send(ErrorPacket("Can't drop/discard item: no inventory"));
			return;
		}

		if (inventory->slotCount <= slot) {
			client.send(ErrorPacket("Can't drop/discard item: invalid slot"));
			return;
		}

		if (discard)
			inventory->erase(slot);
		else
			inventory->drop(slot);
	}
}
