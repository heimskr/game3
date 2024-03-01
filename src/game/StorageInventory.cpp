#include "game/StorageInventory.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	StorageInventory::StorageInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, InventoryID index_, Storage storage_):
		Inventory(std::move(owner), active_slot, index_), storage(std::move(storage_)), slotCount(slot_count) {}

	StorageInventory::StorageInventory(const StorageInventory &other): Inventory() {
		auto lock = other.sharedLock();
		weakOwner = other.weakOwner;
		slotCount = other.slotCount.load();
		activeSlot = other.activeSlot.load();
		index = other.index.load();
		suppressInventoryNotifications = other.suppressInventoryNotifications.load();
		onSwap = other.onSwap;
		onMove = other.onMove;
		storage = other.storage;
	}

	StorageInventory::StorageInventory(StorageInventory &&other): Inventory() {
		auto lock = other.uniqueLock();
		weakOwner = std::move(other.weakOwner);
		slotCount = other.slotCount.exchange(0);
		activeSlot = other.activeSlot.exchange(0);
		index = other.index.load();
		suppressInventoryNotifications = other.suppressInventoryNotifications.exchange(false);
		onSwap = std::move(other.onSwap);
		onMove = std::move(other.onMove);
		storage = std::move(other.storage);
	}

	StorageInventory & StorageInventory::operator=(const StorageInventory &other) {
		if (this == &other)
			return *this;

		Inventory::operator=(other);
		auto this_lock = uniqueLock();
		auto other_lock = other.sharedLock();
		storage = other.storage;
		onSwap = other.onSwap;
		return *this;
	}

	StorageInventory & StorageInventory::operator=(StorageInventory &&other) {
		if (this == &other)
			return *this;

		Inventory::operator=(std::move(other));
		auto this_lock = uniqueLock();
		auto other_lock = other.uniqueLock();
		storage = std::move(other.storage);
		onSwap = std::move(other.onSwap);
		return *this;
	}

	ItemStackPtr StorageInventory::operator[](Slot slot) const {
		if (auto iter = storage.find(slot); iter != storage.end())
			return iter->second;
		return nullptr;
	}

	void StorageInventory::set(Slot slot, ItemStackPtr stack) {
		if (!hasSlot(slot))
			throw std::out_of_range("Slot out of range: " + std::to_string(slot));
		storage[slot] = std::move(stack);
	}

	Slot StorageInventory::getSlotCount() const {
		return slotCount.load();
	}

	void StorageInventory::setSlotCount(Slot new_count) {
		slotCount.store(new_count);
	}

	void StorageInventory::iterate(const std::function<bool(const ItemStackPtr &, Slot)> &function) const {
		for (Slot slot = 0; slot < slotCount; ++slot)
			if (auto iter = storage.find(slot); iter != storage.end())
				if (function(iter->second, slot))
					return;
	}

	ItemStackPtr StorageInventory::firstItem(Slot *slot_out) {
		if (storage.empty()) {
			if (slot_out != nullptr)
				*slot_out = -1;
			return nullptr;
		}

		auto &[slot, stack] = *storage.begin();

		if (slot_out)
			*slot_out = slot;

		return stack;
	}

	ItemStackPtr StorageInventory::firstItem(Slot *slot_out, const std::function<bool(const ItemStackPtr &, Slot)> &predicate) {
		for (auto &[slot, stack]: storage) {
			if (predicate(stack, slot)) {
				if (slot_out)
					*slot_out = slot;
				return stack;
			}
		}

		if (slot_out)
			*slot_out = -1;

		return nullptr;
	}

	bool StorageInventory::canInsert(const ItemStackPtr &stack, const std::function<bool(Slot)> &predicate) const {
		ssize_t remaining = stack->count;

		for (const auto &[slot, stored]: storage) {
			if (!predicate(slot) || !stored->canMerge(*stack))
				continue;
			const ssize_t storable = ssize_t(stored->item->maxCount) - ssize_t(stored->count);
			if (0 < storable) {
				const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}
		}

		if (0 < remaining) {
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (!predicate(slot) || storage.contains(slot))
					continue;
				remaining -= std::min(ItemCount(remaining), stack->item->maxCount);
				if (remaining <= 0)
					break;
			}
		}

		if (remaining < 0)
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");

		return remaining == 0;
	}

	bool StorageInventory::canInsert(const ItemStackPtr &stack, Slot slot) const {
		auto iter = storage.find(slot);

		if (iter == storage.end())
			return stack->count <= stack->item->maxCount;

		const ItemStackPtr &stored = iter->second;

		if (!stored->canMerge(*stack))
			return false;

		ssize_t remaining = stack->count;
		const ssize_t storable = ssize_t(stored->item->maxCount) - ssize_t(stored->count);

		if (0 < storable) {
			const ItemCount to_store = std::min<ItemCount>(remaining, storable);
			remaining -= to_store;
			assert(0 <= remaining);
			return remaining == 0;
		}

		return false;
	}

	bool StorageInventory::canExtract(Slot slot) const {
		return storage.contains(slot);
	}

	ItemCount StorageInventory::insertable(const ItemStackPtr &stack, Slot slot) const {
		auto iter = storage.find(slot);

		if (iter == storage.end())
			return stack->count;

		const ItemStackPtr &stored = iter->second;

		if (!stored->canMerge(*stack))
			return 0;

		return stored->item->maxCount - stack->count;
	}

	ItemCount StorageInventory::count(const ItemID &id) const {
		if (id.getPath() == "attribute")
			return countAttribute(id);

		ItemCount out = 0;

		for (const auto &[slot, stack]: storage)
			if (stack->item->identifier == id)
				out += stack->count;

		return out;
	}

	ItemCount StorageInventory::count(const Item &item) const {
		ItemCount out = 0;

		for (const auto &[slot, stack]: storage)
			if (stack->item->identifier == item.identifier)
				out += stack->count;

		return out;
	}

	ItemCount StorageInventory::count(const ItemStackPtr &stack) const {
		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (stack->canMerge(*stored_stack))
				out += stored_stack->count;

		return out;
	}

	ItemCount StorageInventory::count(const ItemStackPtr &stack, const std::function<bool(Slot)> &predicate) const {
		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (predicate(slot) && stack->canMerge(*stored_stack))
				out += stored_stack->count;

		return out;
	}

	ItemCount StorageInventory::countAttribute(const Identifier &attribute) const {
		ItemCount out = 0;

		for (const auto &[slot, stack]: storage)
			if (stack->hasAttribute(attribute))
				out += stack->count;

		return out;
	}

	bool StorageInventory::hasSlot(Slot slot) const {
		return 0 <= slot && slot < slotCount;
	}

	bool StorageInventory::empty() const {
		return storage.empty();
	}

	ItemStackPtr StorageInventory::front() const {
		if (storage.empty())
			throw std::out_of_range("Inventory empty");
		return storage.begin()->second;
	}

	bool StorageInventory::contains(Slot slot) const {
		return storage.contains(slot);
	}

	std::optional<Slot> StorageInventory::find(const ItemID &id, const Predicate &predicate) const {
		for (const auto &[slot, stack]: storage)
			if (predicate(stack, slot) && stack->item->identifier == id)
				return slot;
		return std::nullopt;
	}

	std::optional<Slot> StorageInventory::findAttribute(const Identifier &attribute, const Predicate &predicate) const {
		for (const auto &[slot, stack]: storage)
			if (predicate(stack, slot) && stack->item->attributes.contains(attribute))
				return slot;
		return std::nullopt;
	}

	ItemStackPtr StorageInventory::getActive() const {
		if (auto iter = storage.find(activeSlot); iter != storage.end())
			return iter->second;
		return nullptr;
	}

	bool StorageInventory::contains(const ItemStackPtr &needle, const Predicate &predicate) const {
		ItemCount remaining = needle->count;
		for (const auto &[slot, stack]: storage) {
			if (!predicate(stack, slot) || !needle->canMerge(*stack))
				continue;
			if (remaining <= stack->count)
				return true;
			remaining -= stack->count;
		}

		return false;
	}

	void StorageInventory::prevSlot() {
		if (0 < activeSlot)
			setActive(activeSlot - 1, false);
	}

	void StorageInventory::nextSlot() {
		if (activeSlot < slotCount - 1)
			setActive(activeSlot + 1, false);
	}

	void StorageInventory::replace(const Inventory &other) {
		if (this == &other)
			return;

		if (const auto *other_storage = dynamic_cast<const StorageInventory *>(&other)) {
			*this = *other_storage;
		} else {
			throw std::invalid_argument("StorageInventory::replace expected a StorageInventory");
		}
	}

	void StorageInventory::replace(Inventory &&other) {
		if (this == &other)
			return;

		if (auto *other_storage = dynamic_cast<StorageInventory *>(&other)) {
			*this = std::move(*other_storage);
		} else {
			throw std::invalid_argument("StorageInventory::replace expected a StorageInventory");
		}
	}

	void StorageInventory::compact() {
		auto lock = storage.uniqueLock();

		for (auto iter = storage.begin(); iter != storage.end();) {
			if (iter->second->count == 0)
				storage.erase(iter++);
			else
				++iter;
		}
	}
}
