#include "game/Game.h"
#include "game/ClientInventory.h"
#include "game/HasInventory.h"
#include "game/ServerInventory.h"
#include "net/Buffer.h"

namespace Game3 {
	void HasInventory::setInventory(std::shared_ptr<Inventory> new_inventory) {
		inventory = std::move(new_inventory);
	}

	void HasInventory::encode(Buffer &buffer) {
		std::shared_ptr<ServerInventory> server_inventory;
		if (inventory) {
			server_inventory = std::dynamic_pointer_cast<ServerInventory>(inventory);
			assert(server_inventory);
			buffer << inventory->slotCount.load();
		} else
			buffer << Slot(-1);
		buffer << server_inventory;
	}

	namespace {
		template <typename T>
		void decodeHasInventory(HasInventory &has_inventory, Buffer &buffer) {
			Slot slot_count = -1;
			buffer >> slot_count;
			std::shared_ptr<T> inventory;
			if (slot_count == -1) {
				has_inventory.setInventory(nullptr);
				has_inventory.inventoryUpdated();
				buffer >> inventory;
				assert(!inventory);
			} else {
				buffer >> inventory;
				assert(inventory);
				if (inventory) { // This is unnecessary but I want PVS-Studio to be happy.
					inventory->weakOwner = has_inventory.getSharedAgent();
					inventory->slotCount = slot_count; // Maybe not necessary? Try an assert before.
					has_inventory.setInventory(inventory);
					has_inventory.inventoryUpdated();
				}
			}
		}
	}

	void HasInventory::decode(Buffer &buffer) {
		if (getSharedAgent()->getSide() == Side::Client)
			decodeHasInventory<ClientInventory>(*this, buffer);
		else
			decodeHasInventory<ServerInventory>(*this, buffer);
	}
}