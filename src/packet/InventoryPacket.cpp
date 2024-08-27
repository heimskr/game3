#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/Inventory.h"
#include "game/ServerInventory.h"
#include "packet/InventoryPacket.h"

namespace Game3 {
	void InventoryPacket::encode(Game &, Buffer &buffer) const {
		buffer << *std::dynamic_pointer_cast<ServerInventory>(inventory);
	}

	void InventoryPacket::decode(Game &, Buffer &buffer) {
		inventory = std::make_shared<ClientInventory>(buffer.take<ClientInventory>());
	}

	void InventoryPacket::handle(const ClientGamePtr &game) {
		if (PlayerPtr player = game->getPlayer()) {
			inventory->weakOwner = player;
			player->setInventory(inventory, 0);
			player->getInventory(0)->notifyOwner();
		} else {
			ERROR("InventoryPacket::handle: player is missing");
			assert(player);
		}
	}
}
