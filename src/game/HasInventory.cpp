#include "game/Game.h"
#include "game/HasInventory.h"
#include "game/Inventory.h"
#include "net/Buffer.h"

namespace Game3 {
	void HasInventory::encode(Buffer &buffer) {
		std::optional<std::shared_ptr<Inventory>> optional;
		if (inventory) {
			optional = inventory;
			buffer << inventory->slotCount;
		} else
			buffer << static_cast<Slot>(-1);
		buffer << optional;
	}

	void HasInventory::decode(Buffer &buffer) {
		Slot slot_count = -1;
		buffer >> slot_count;
		std::optional<Inventory> optional;
		if (slot_count == -1) {
			inventory.reset();
			buffer >> optional;
			assert(!optional);
		} else {
			buffer >> optional;
			assert(optional);
			if (optional) { // This is unnecessary but I want PVS-Studio to be happy.
				inventory = std::make_shared<Inventory>(std::move(*optional));
				inventory->weakOwner = getSharedAgent();
				inventory->slotCount = slot_count; // Maybe not necessary? Try an assert before.
			}
		}
	}
}