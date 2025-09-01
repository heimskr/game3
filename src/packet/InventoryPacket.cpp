#include "util/Log.h"
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

	void InventoryPacket::decode(Game &, BasicBuffer &buffer) {
		inventory = std::make_shared<ClientInventory>(buffer.take<ClientInventory>());
	}

	void InventoryPacket::handle(const ClientGamePtr &game) {
		if (game->isConnectedLocally()) {
			// TODO: transmogrify ServerInventory into ClientInventory without going through a Buffer.
			Buffer buffer{game, Side::Client};
			encode(*game, buffer);
			decode(*game, buffer);
		}

		if (PlayerPtr player = game->getPlayer()) {
			inventory->setOwner(player);
			player->setInventory(inventory, 0);
			player->getInventory(0)->notifyOwner({});
		} else {
			ERR("InventoryPacket::handle: player is missing");
			assert(player);
		}
	}
}
