#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "packet/ActiveSlotSetPacket.h"

namespace Game3 {
	void ActiveSlotSetPacket::handle(ClientGame &client) {
		client.player->getInventory()->setActive(slot, true);
	}
}
