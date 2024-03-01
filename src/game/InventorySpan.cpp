#include "game/InventorySpan.h"

namespace Game3 {
	InventorySpan::InventorySpan(std::shared_ptr<Inventory> inventory_, SlotRange range_):
		InventoryWrapper(std::move(inventory_)), range(range_) {}

	InventorySpan::InventorySpan(std::shared_ptr<Inventory> inventory_, Slot min, Slot max):
		InventoryWrapper(std::move(inventory_)), range{min, max} {}

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

	Slot InventorySpan::getSlotCount() const {
		return range.size();
	}

	void InventorySpan::setSlotCount(Slot new_count) {
		if (new_count != getSlotCount())
			throw std::runtime_error("Cannot resize InventorySpan");
	}

	std::unique_ptr<Inventory> InventorySpan::copy() const {
		return std::make_unique<InventorySpan>(inventory->copy(), range);
	}

	bool InventorySpan::empty() const {
		if (range.min == 0 && range.max == inventory->getSlotCount())
			return inventory->empty();

		for (Slot slot = range.min; slot <= range.max; ++slot)
			if (inventory->contains(slot))
				return false;

		return true;
	}

	void InventorySpan::replace(const Inventory &other) {
		if (other.getSlotCount() != getSlotCount())
			throw std::invalid_argument("Invalid slot count");

		for (Slot slot = range.min; slot <= range.max; ++slot) {
			if (ItemStackPtr stack = other[slot])
				set(slot, stack);
			else
				erase(slot);
		}
	}

	void InventorySpan::replace(Inventory &&other) {
		if (other.getSlotCount() != getSlotCount())
			throw std::invalid_argument("Invalid slot count");

		for (Slot slot = range.min; slot <= range.max; ++slot) {
			if (ItemStackPtr stack = other[slot])
				set(slot, std::move(stack));
			else
				erase(slot);
		}
	}
}
