#include "game/Inventory.h"
#include "item/Item.h"
#include "net/Buffer.h"
#include "pipes/ItemFilter.h"

#include <algorithm>

namespace Game3 {
	namespace {
		bool isMatch(const ItemStackPtr &stack, std::reference_wrapper<const Inventory> source, bool strict, const ItemFilter::Config &config) {
			return config(stack, source, strict);
		}
	}

	ItemFilter::ItemFilter() = default;

	ItemFilter::ItemFilter(bool allow_mode, bool strict_):
		allowMode(allow_mode),
		strict(strict_) {}

	bool ItemFilter::isAllowed(const ItemStackPtr &stack, const Inventory &inventory) const {
		auto lock = configsByItem.sharedLock();

		if (auto iter = configsByItem.find(stack->item->identifier); iter != configsByItem.end()) {
			if (std::ranges::any_of(iter->second, std::bind_front(&isMatch, std::cref(stack), std::cref(inventory), strict))) {
				return allowMode;
			}
		}

		return !allowMode;
	}

	void ItemFilter::addItem(const ItemStackPtr &stack) {
		auto items_lock = items.uniqueLock();
		auto configs_lock = configsByItem.uniqueLock();
		items.insert(stack->item->identifier);
		configsByItem[stack->item->identifier].emplace(stack->data);
	}

	void ItemFilter::removeItem(const ItemStackPtr &stack) {
		auto items_lock = items.uniqueLock();
		auto configs_lock = configsByItem.uniqueLock();

		if (auto iter = configsByItem.find(stack->item->identifier); iter != configsByItem.end()) {
			auto &set = iter->second;

			std::erase_if(set, [&](const Config &config) {
				return config.data == stack->data;
			});

			if (set.empty()) {
				configsByItem.erase(iter);
				items.erase(stack->item->identifier);
			}
		} else {
			items.erase(stack->item->identifier);
		}
	}

	void ItemFilter::clear() {
		auto items_lock = items.uniqueLock();
		auto configs_lock = configsByItem.uniqueLock();
		items.clear();
		configsByItem.clear();
	}

	bool ItemFilter::Config::operator()(const ItemStackPtr &stack, const Inventory &inventory, bool strict) const {
		if (strict && stack->data != data) {
			return false;
		}

		if (comparator != Comparator::None) {
			const ItemCount inventory_count = strict? inventory.count(stack) : inventory.count(*stack->item);
			if (comparator == Comparator::Less) {
				return inventory_count < count;
			}
			return inventory_count > count;
		}

		return true;
	}

	bool ItemFilter::Config::operator==(const Config &other) const {
		return this == &other || (count == other.count && comparator == other.comparator && data == other.data);
	}

	template <>
	std::string Buffer::getType(const ItemFilter::Config &, bool) {
		return {'\xe4'};
	}

	template <>
	ItemFilter::Config popBuffer<ItemFilter::Config>(Buffer &buffer) {
		ItemFilter::Config out;
		buffer >> out;
		return out;
	}

	Buffer & operator+=(Buffer &buffer, const ItemFilter::Config &config) {
		return buffer << config;
	}

	Buffer & operator<<(Buffer &buffer, const ItemFilter::Config &config) {
		buffer.appendType(config, false);
		return buffer << config.data << config.comparator << config.count;
	}

	Buffer & operator>>(Buffer &buffer, ItemFilter::Config &config) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(config, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected ItemFilter::Config)");
		}
		return buffer >> config.data >> config.comparator >> config.count;
	}

	template <>
	std::string Buffer::getType(const ItemFilter &, bool) {
		return {'\xe3'};
	}

	Buffer & operator+=(Buffer &buffer, const ItemFilter &filter) {
		return buffer << filter;
	}

	Buffer & operator<<(Buffer &buffer, const ItemFilter &filter) {
		buffer.appendType(filter, false);
		return buffer << filter.allowMode << filter.strict << filter.items << filter.configsByItem;
	}

	Buffer & operator>>(Buffer &buffer, ItemFilter &filter) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(filter, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected ItemFilter)");
		}
		return buffer >> filter.allowMode >> filter.strict >> filter.items >> filter.configsByItem;
	}
}
