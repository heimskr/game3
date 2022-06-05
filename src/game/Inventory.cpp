#include "game/Inventory.h"

namespace Game3 {
	ItemStack * Inventory::operator[](size_t slot) {
		if (storage.contains(slot))
			return &storage.at(slot);
		return nullptr;
	}

	const ItemStack * Inventory::operator[](size_t slot) const {
		if (storage.contains(slot))
			return &storage.at(slot);
		return nullptr;
	}

	std::unique_ptr<ItemStack> Inventory::add(const ItemStack &stack) {
		int remaining = stack.count;

		for (auto &[slot, stored]: storage) {
			if (!stored.canMerge(stack))
				continue;
			const int storable = int(stored.item->maxCount) - int(stored.count);
			if (0 < storable) {
				const int to_store = std::min(remaining, storable);
				stored.count += to_store;
				remaining -= to_store;
				if (remaining == 0)
					break;
			}
		}

		for (Slot slot = 0; slot < slotCount; ++slot) {
			if (storage.contains(slot))
				continue;
			const unsigned to_store = std::min(unsigned(remaining), stack.item->maxCount);
			storage.emplace(slot, ItemStack(stack.item, to_store));
			remaining -= to_store;
			if (remaining == 0)
				break;
		}

		return remaining == 0? nullptr : std::make_unique<ItemStack>(stack.item, remaining);
	}

	void to_json(nlohmann::json &json, const Inventory &inventory) {
		json["storage"] = inventory.storage;
		json["slotCount"] = inventory.slotCount;
	}

	void from_json(const nlohmann::json &json, Inventory &inventory) {
		inventory.storage = json.at("storage");
		inventory.slotCount = json.at("slotCount");
	}
}
