#include "game/ExpandedServerInventory.h"
#include "item/Item.h"

namespace Game3 {
	std::unique_ptr<Inventory> ExpandedServerInventory::copy() const {
		return std::make_unique<ExpandedServerInventory>(*this);
	}

	std::optional<ItemStack> ExpandedServerInventory::add(const ItemStack &stack, const std::function<bool(Slot)> &predicate, Slot start) {
		bool done = false;

		INFO("ExpandedServerInventory::add({}): storage = {}", stack, nlohmann::json(storage).dump());

		// TODO: avoid overflow or whatever, in case someone tries to store more than 19+ quintillion of something

		if (0 <= start) {
			if (slotCount <= start)
				throw std::out_of_range(std::format("Can't start at slot {}: out of range", start));

			if (predicate(start)){
				if (auto iter = storage.find(start); iter != storage.end()) {
					ItemStack &stored = iter->second;
					stored.count += stack.count;
					done = true;
					INFO("{}:{}", __FILE__, __LINE__);
				}
			}
		}

		if (!done) {
			for (auto &[slot, stored]: storage) {
				if (slot == start || !stored.canMerge(stack) || !predicate(slot)) {
					INFO("{}:{}: {}, {}, {}", __FILE__, __LINE__, slot == start, !stored.canMerge(stack), !predicate(slot));
					continue;
				}
				stored.count += stack.count;
				done = true;
				INFO("{}:{}", __FILE__, __LINE__);
				break;
			}
		}

		if (!done) {
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot) || !predicate(slot)) {
					INFO("{}:{}: {}, {}", __FILE__, __LINE__, storage.contains(slot), !predicate(slot));
					continue;
				}
				storage.emplace(slot, stack);
				done = true;
				INFO("{}:{}", __FILE__, __LINE__);
				break;
			}
		}

		if (done) {
			INFO("{}:{}: {}", __FILE__, __LINE__, nlohmann::json(storage).dump());
			notifyOwner();
			return std::nullopt;
		}

		INFO("{}:{}", __FILE__, __LINE__);
		return std::make_optional(stack);
	}
}
