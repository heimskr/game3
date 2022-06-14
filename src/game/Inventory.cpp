#include <iostream>

#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "game/HasRealm.h"
#include "game/Inventory.h"
#include "realm/Realm.h"

namespace Game3 {
	Inventory::Inventory(const std::shared_ptr<HasRealm> &owner_, Slot slot_count):
		owner(owner_), slotCount(slot_count) {}

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

	std::optional<ItemStack> Inventory::add(const ItemStack &stack, Slot start) {
		ssize_t remaining = stack.count;

		if (0 <= start) {
			if (slotCount <= start)
				throw std::out_of_range("Can't start at slot " + std::to_string(start) + ": out of range");
			if (storage.contains(start)) {
				auto &stored = storage.at(start);
				if (stored.canMerge(stack)) {
					const ssize_t storable = ssize_t(stored.item->maxCount) - ssize_t(stored.count);
					if (0 < storable) {
						const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
						stored.count += to_store;
						remaining -= to_store;
					}
				}
			} else {
				const ItemCount to_store = std::min(ItemCount(stack.item->maxCount), ItemCount(remaining));
				storage.try_emplace(start, stack.item, to_store);
				remaining -= to_store;
			}
		}

		if (0 < remaining)
			for (auto &[slot, stored]: storage) {
				if (slot == start || !stored.canMerge(stack))
					continue;
				const ssize_t storable = ssize_t(stored.item->maxCount) - ssize_t(stored.count);
				if (0 < storable) {
					const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
					stored.count += to_store;
					remaining -= to_store;
					if (remaining <= 0)
						break;
				}
			}

		if (0 < remaining)
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot))
					continue;
				const ItemCount to_store = std::min(ItemCount(remaining), stack.item->maxCount);
				storage.emplace(slot, ItemStack(stack.item, to_store, stack.data));
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}

		notifyOwner();

		if (remaining < 0)
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");

		if (remaining == 0)
			return std::nullopt;

		return ItemStack(stack.item, remaining);
	}

	bool Inventory::canStore(const ItemStack &stack) const {
		ssize_t remaining = stack.count;

		for (const auto &[slot, stored]: storage) {
			if (!stored.canMerge(stack))
				continue;
			const ssize_t storable = ssize_t(stored.item->maxCount) - ssize_t(stored.count);
			if (0 < storable) {
				const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}
		}

		if (0 < remaining)
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot))
					continue;
				remaining -= std::min(ItemCount(remaining), stack.item->maxCount);
				if (remaining <= 0)
					break;
			}

		if (remaining < 0)
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");

		return remaining == 0;
	}

	void Inventory::drop(Slot slot) {
		if (!storage.contains(slot))
			return;

		auto locked_owner = owner.lock();
		if (!locked_owner)
			throw std::logic_error("Inventory is missing an owner");

		auto realm = locked_owner->getRealm();
		realm->spawn<ItemEntity>(locked_owner->getPosition(), storage.at(slot));
		storage.erase(slot);

		notifyOwner();
	}

	bool Inventory::swap(Slot source, Slot destination) {
		if (slotCount <= source || slotCount <= destination || !storage.contains(source))
			return false;

		if (storage.contains(destination)) {
			std::swap(storage.at(source), storage.at(destination));
		} else {
			storage[destination] = storage.at(source);
			storage.erase(source);
		}

		notifyOwner();
		return true;
	}

	void Inventory::erase(Slot slot) {
		storage.erase(slot);
		notifyOwner();
	}

	ItemCount Inventory::count(const Item &item) const {
		ItemCount out = 0;
		for (const auto &[slot, stored_item]: storage)
			if (stored_item.item->id == item.id)
				out += stored_item.count;
		return out;
	}

	ItemCount Inventory::count(const ItemStack &stack) const {
		return count(*stack.item);
	}

	std::shared_ptr<HasRealm> Inventory::getOwner() const {
		if (auto locked_owner = owner.lock())
			return locked_owner;
		throw std::runtime_error("Couldn't lock inventory owner");
	}

	ItemStack & Inventory::front() {
		if (storage.empty())
			throw std::out_of_range("Inventory empty");
		return storage.begin()->second;
	}

	const ItemStack & Inventory::front() const {
		if (storage.empty())
			throw std::out_of_range("Inventory empty");
		return storage.begin()->second;
	}

	ItemCount Inventory::remove(const ItemStack &stack_to_remove) {
		bool retry;
		ItemCount count_to_remove = stack_to_remove.count;
		ItemCount removed = 0;
		do {
			retry = false;
			for (auto &[slot, stack]: storage) {
				if (stack.canMerge(stack_to_remove)) {
					const auto to_remove = std::min(stack.count, count_to_remove);
					stack.count -= to_remove;
					count_to_remove -= to_remove;
					removed += 0;
					if (stack.count == 0) {
						// Erasing from a map can invalidate iterators.
						storage.erase(slot);
						retry = true;
						break;
					}

					if (count_to_remove == 0)
						return removed;
				}
			}

			if (count_to_remove == 0)
				return removed;
		} while (retry);

		return removed;
	}

	ItemCount Inventory::remove(const ItemStack &stack_to_remove, Slot slot) {
		auto &stack = storage.at(slot);
		if (!stack_to_remove.canMerge(stack))
			return 0;

		const auto to_remove = std::min(stack.count, stack_to_remove.count);
		if ((stack.count -= to_remove) == 0)
			storage.erase(slot);

		return to_remove;
	}

	bool Inventory::contains(Slot slot) const {
		return storage.contains(slot);
	}

	std::optional<Slot> Inventory::find(ItemID id) const {
		for (const auto &[slot, stack]: storage)
			if (stack.item->id == id)
				return slot;
		return std::nullopt;
	}

	bool Inventory::contains(const ItemStack &needle) const {
		ItemCount remaining = needle.count;
		for (const auto &[slot, stack]: storage) {
			if (!needle.canMerge(stack))
				continue;
			if (remaining <= stack.count)
				return true;
			remaining -= stack.count;
		}

		return false;
	}

	void Inventory::notifyOwner() {
		if (auto locked_owner = owner.lock()) {
			if (auto player = std::dynamic_pointer_cast<Player>(locked_owner))
				player->getRealm()->getGame().signal_player_inventory_update().emit(player);
			else
				locked_owner->getRealm()->getGame().signal_other_inventory_update().emit(locked_owner);
		}
	}

	Inventory Inventory::fromJSON(const nlohmann::json &json, const std::shared_ptr<HasRealm> &owner) {
		Inventory out(owner, 0);
		out.storage = json.at("storage");
		out.slotCount = json.at("slotCount");
		return out;
	}

	void to_json(nlohmann::json &json, const Inventory &inventory) {
		json["storage"] = inventory.storage;
		json["slotCount"] = inventory.slotCount;
	}
}
