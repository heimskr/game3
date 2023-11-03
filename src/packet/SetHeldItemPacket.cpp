#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SetHeldItemPacket.h"

namespace Game3 {
	void SetHeldItemPacket::handle(ServerGame &, RemoteClient &client) {
		const ServerPlayerPtr player = client.getPlayer();
		if (!player) {
			client.send(ErrorPacket("Can't set held item: no player"));
			return;
		}

		const InventoryPtr inventory = player->getInventory(0);
		if (!inventory) {
			client.send(ErrorPacket("Can't set held item: no inventory"));
			return;
		}

		if (inventory->slotCount <= slot) {
			client.send(ErrorPacket("Can't set held item: invalid slot"));
			return;
		}

		if (leftHand)
			player->setHeldLeft(player->getHeldLeft() == slot? -1 : slot);
		else
			player->setHeldRight(player->getHeldRight() == slot? -1 : slot);
	}
}
