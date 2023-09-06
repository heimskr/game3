#include "game/Game.h"
#include "game/ClientInventory.h"
#include "game/HasInventory.h"
#include "game/ServerInventory.h"
#include "net/Buffer.h"

namespace Game3 {
	void HasInventory::encode(Buffer &buffer) {
		std::optional<std::shared_ptr<ServerInventory>> optional;
		if (inventory) {
			auto server_inventory = std::dynamic_pointer_cast<ServerInventory>(inventory);
			assert(server_inventory);
			optional = server_inventory;
			buffer << inventory->slotCount.load();
		} else
			buffer << Slot(-1);
		buffer << optional;
	}

	namespace {
		template <typename T>
		void decodeHasInventory(HasInventory &has_inventory, Buffer &buffer) {
			Slot slot_count = -1;
			buffer >> slot_count;
			std::optional<T> optional;
			if (slot_count == -1) {
				has_inventory.inventory.reset();
				has_inventory.inventoryUpdated();
				buffer >> optional;
				assert(!optional);
			} else {
				buffer >> optional;
				assert(optional);
				if (optional) { // This is unnecessary but I want PVS-Studio to be happy.
					has_inventory.inventory = std::make_shared<T>(std::move(*optional));
					has_inventory.inventory->weakOwner = has_inventory.getSharedAgent();
					has_inventory.inventory->slotCount = slot_count; // Maybe not necessary? Try an assert before.
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