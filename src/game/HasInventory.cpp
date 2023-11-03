#include "game/Game.h"
#include "game/ClientInventory.h"
#include "game/HasInventory.h"
#include "game/ServerInventory.h"
#include "net/Buffer.h"

namespace Game3 {
	const std::shared_ptr<Inventory> & HasInventory::getInventory(InventoryID inventory_id) const {
		if (inventory_id != 0)
			throw std::invalid_argument("Couldn't retrieve inventory with index " + std::to_string(inventory_id));
		return inventory;
	}

	void HasInventory::setInventory(std::shared_ptr<Inventory> new_inventory, InventoryID inventory_id) {
		if (inventory_id != 0)
			throw std::invalid_argument("Couldn't retrieve inventory with index " + std::to_string(inventory_id));
		inventory = std::move(new_inventory);
	}

	void HasInventory::encode(Buffer &buffer, InventoryID index) {
		std::shared_ptr<ServerInventory> server_inventory;
		InventoryPtr subinventory = getInventory(index);
		if (subinventory) {
			server_inventory = std::dynamic_pointer_cast<ServerInventory>(subinventory);
			assert(server_inventory);
			buffer << subinventory->slotCount.load();
		} else
			buffer << Slot(-1);
		buffer << server_inventory;
	}

	namespace {
		template <typename T>
		void decodeHasInventory(HasInventory &has_inventory, Buffer &buffer, InventoryID index) {
			Slot slot_count = -1;
			buffer >> slot_count;
			std::shared_ptr<T> inventory;
			if (slot_count == -1) {
				has_inventory.setInventory(nullptr, index);
				has_inventory.inventoryUpdated();
				buffer >> inventory;
				assert(!inventory);
			} else {
				buffer >> inventory;
				assert(inventory);
				if (inventory) { // This is unnecessary but I want PVS-Studio to be happy.
					inventory->weakOwner = has_inventory.getSharedAgent();
					inventory->slotCount = slot_count; // Maybe not necessary? Try an assert before.
					inventory->index = index;
					has_inventory.setInventory(inventory, index);
					has_inventory.inventoryUpdated();
				}
			}
		}
	}

	void HasInventory::decode(Buffer &buffer, InventoryID index) {
		if (getSharedAgent()->getSide() == Side::Client)
			decodeHasInventory<ClientInventory>(*this, buffer, index);
		else
			decodeHasInventory<ServerInventory>(*this, buffer, index);
	}
}