#include "game/InventorySpan.h"

namespace Game3 {
	InventorySpan::InventorySpan(std::shared_ptr<Inventory> inventory_, SlotRange range_):
	InventoryWrapper(std::move(inventory_)), range(range_) {
		slotCount = range.size();
	}

	InventorySpan::InventorySpan(std::shared_ptr<Inventory> inventory_, Slot min, Slot max):
	InventoryWrapper(std::move(inventory_)), range{min, max} {
		slotCount = range.size();
	}

	bool InventorySpan::validateSlot(Slot slot) const {
		return range.contains(slot);
	}

	Slot InventorySpan::adjustSlot(Slot slot) const {
		if (slot < range.min)
			return range.min;
		if (slot > range.max)
			return range.max;
		return slot;
	}

	std::unique_ptr<Inventory> InventorySpan::copy() const {
		return std::make_unique<InventorySpan>(inventory->copy(), range);
	}

	bool InventorySpan::empty() const {
		if (range.min == 0 && range.max == inventory->slotCount)
			return inventory->empty();

		for (Slot slot = range.min; slot <= range.max; ++slot)
			if (inventory->contains(slot))
				return false;

		return true;
	}
}
