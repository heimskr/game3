#include "item/Item.h"
#include "pipes/ItemFilter.h"

namespace Game3 {
	ItemFilter::ItemFilter() = default;

	ItemFilter::ItemFilter(bool allow_mode, bool strict_):
		allowMode(allow_mode),
		strict(strict_) {}

	bool ItemFilter::isAllowed(const ItemStack &stack) const {
		auto lock = dataByItem.sharedLock();

		if (strict) {
			if (auto iter = dataByItem.find(stack.item->identifier); iter != dataByItem.end() && iter->second.contains(stack.data))
				return allowMode;
			return !allowMode;
		}

		if (dataByItem.contains(stack.item->identifier))
			return allowMode;

		return !allowMode;
	}

	void ItemFilter::addItem(const ItemStack &stack) {
		auto items_lock = items.uniqueLock();
		auto data_lock = dataByItem.uniqueLock();
		items.insert(stack.item->identifier);
		dataByItem[stack.item->identifier].insert(stack.data);
	}

	void ItemFilter::removeItem(const ItemStack &stack) {
		auto items_lock = items.uniqueLock();
		auto data_lock = dataByItem.uniqueLock();

		if (auto iter = dataByItem.find(stack.item->identifier); iter != dataByItem.end()) {
			auto &set = iter->second;
			set.erase(stack.data);
			if (set.empty()) {
				dataByItem.erase(iter);
				items.erase(stack.item->identifier);
			}
		} else {
			items.erase(stack.item->identifier);
		}
	}

	void ItemFilter::clear() {
		auto items_lock = items.uniqueLock();
		auto data_lock = dataByItem.uniqueLock();
		items.clear();
		dataByItem.clear();
	}
}
