#include "Log.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "packet/InventoryPacket.h"

namespace Game3 {
	void InventoryPacket::encode(Game &, Buffer &buffer) const {
		buffer << *inventory;
	}

	void InventoryPacket::decode(Game &, Buffer &buffer) {
		inventory = std::make_shared<Inventory>(buffer.take<Inventory>());
	}

	void InventoryPacket::handle(ClientGame &game) {
		if (game.player) {
			inventory->weakOwner = game.player;
			game.player->inventory = inventory;
			game.player->inventory->notifyOwner();
		} else {
			ERROR("InventoryPacket::handle: player is missing");
			assert(game.player);
		}
	}
}
