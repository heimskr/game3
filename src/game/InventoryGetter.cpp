#include "game/Agent.h"
#include "game/HasInventory.h"
#include "game/Inventory.h"
#include "game/InventoryGetter.h"

namespace Game3 {
	InventoryGetter::InventoryGetter(const Inventory &inventory):
		owner(inventory.getOwner()),
		index(inventory.index) {}

	std::shared_ptr<Inventory> InventoryGetter::get() const {
		if (auto haver = std::dynamic_pointer_cast<HasInventory>(owner))
			return haver->getInventory(index);

		throw std::runtime_error("Couldn't cast inventory owner to HasInventory");
	}

	InventoryGetter::operator std::shared_ptr<Inventory>() const {
		return get();
	}
}
