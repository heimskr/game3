#include "game/Game.h"
#include "game/ClientInventory.h"
#include "game/HasInventory.h"
#include "game/ServerInventory.h"
#include "net/Buffer.h"

namespace Game3 {
	const std::shared_ptr<Inventory> & HasInventory::getInventory(InventoryID inventory_id) const {
		if (inventory_id != 0)
			throw std::invalid_argument(std::format("Couldn't retrieve inventory with index {}", inventory_id));
		return inventory;
	}

	void HasInventory::setInventory(std::shared_ptr<Inventory> new_inventory, InventoryID inventory_id) {
		if (inventory_id != 0)
			throw std::invalid_argument(std::format("Couldn't set inventory with index {}", inventory_id));
		inventory = std::move(new_inventory);
	}

	void HasInventory::encode(Buffer &buffer, InventoryID index) {
		std::shared_ptr<ServerInventory> server_inventory;
		InventoryPtr subinventory = getInventory(index);
		if (subinventory) {
			server_inventory = std::dynamic_pointer_cast<ServerInventory>(subinventory);
			assert(server_inventory);
			buffer << subinventory->getSlotCount();
		} else {
			buffer << static_cast<Slot>(-1);
		}
		buffer << server_inventory;
	}

	void HasInventory::decode(Buffer &buffer, InventoryID index) {
		if (getSharedAgent()->getSide() == Side::Client) {
			decodeSpecific<ClientInventory>(buffer, index);
		} else {
			decodeSpecific<ServerInventory>(buffer, index);
		}
	}
}