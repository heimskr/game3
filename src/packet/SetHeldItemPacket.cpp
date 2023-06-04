#include "entity/ServerPlayer.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/SetHeldItemPacket.h"

namespace Game3 {
	void SetHeldItemPacket::handle(ServerGame &, RemoteClient &client) {
		auto player = client.getPlayer();
		if (!player) {
			client.send(ErrorPacket("Can't set held item: no player"));
			return;
		}

		auto inventory = player->inventory;
		if (!inventory) {
			client.send(ErrorPacket("Can't set held item: no inventory"));
			return;
		}

		if (inventory->slotCount <= slot) {
			client.send(ErrorPacket("Can't set held item: invalid slot"));
			return;
		}

		if (leftHand)
			player->setHeldLeft(slot);
		else
			player->setHeldRight(slot);
	}
}
