#include "game/ExpandedServerInventory.h"
#include "item/Item.h"

namespace Game3 {
	std::unique_ptr<Inventory> ExpandedServerInventory::copy() const {
		return std::make_unique<ExpandedServerInventory>(*this);
	}

	ItemStackPtr ExpandedServerInventory::add(const ItemStackPtr &stack, const std::function<bool(Slot)> &predicate, Slot start) {
		bool done = false;

		// TODO: avoid overflow or whatever, in case someone tries to store more than 19+ quintillion of something

		if (0 <= start) {
			if (slotCount <= start) {
				throw std::out_of_range(std::format("Can't start at slot {}: out of range", start));
			}

			if (predicate(start)) {
				if (auto iter = storage.find(start); iter != storage.end()) {
					const ItemStackPtr &stored = iter->second;
					stored->count += stack->count;
					done = true;
				}
			}
		}

		if (!done) {
			for (auto &[slot, stored]: storage) {
				if (slot == start || !stored->canMerge(*stack) || !predicate(slot)) {
					continue;
				}

				stored->count += stack->count;
				done = true;
				break;
			}
		}

		if (!done) {
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot) || !predicate(slot)) {
					continue;
				}

				storage.emplace(slot, stack);
				done = true;
				break;
			}
		}

		if (done) {
			notifyOwner(stack);
			return nullptr;
		}

		return stack;
	}

	bool ExpandedServerInventory::canInsert(const ItemStackPtr &stack, const std::function<bool(Slot)> &predicate) const {
		for (Slot slot = 0; slot < slotCount; ++slot) {
			if (!predicate(slot)) {
				continue;
			}

			if (auto iter = storage.find(slot); iter != storage.end()) {
				if (iter->second->canMerge(*stack)) {
					return true;
				}
			} else {
				return true;
			}
		}

		return false;
	}

	bool ExpandedServerInventory::canInsert(const ItemStackPtr &stack, Slot slot) const {
		auto iter = storage.find(slot);
		if (iter == storage.end()) {
			return true;
		}
		return iter->second->canMerge(*stack);
	}

	template <>
	std::string BasicBuffer::getType(const ExpandedServerInventory &, bool) {
		return "\xe1";
	}

	Buffer & operator+=(Buffer &buffer, const ExpandedServerInventory &inventory) {
		return buffer += static_cast<const ServerInventory &>(inventory);
	}

	Buffer & operator<<(Buffer &buffer, const ExpandedServerInventory &inventory) {
		return buffer += static_cast<const ServerInventory &>(inventory);
	}

	BasicBuffer & operator>>(BasicBuffer &buffer, ExpandedServerInventory &inventory) {
		return buffer >> static_cast<ServerInventory &>(inventory);
	}
}
