#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/SetActiveSlotPacket.h"

namespace Game3 {
	void SetActiveSlotPacket::handle(ServerGame &, RemoteClient &client) {
		if (const PlayerPtr player = client.getPlayer())
			if (const InventoryPtr inventory = player->getInventory())
				inventory->setActive(slot);
	}

	void SetActiveSlotPacket::handle(ClientGame &game) {
		game.player->getInventory()->setActive(slot, true);
	}
}
