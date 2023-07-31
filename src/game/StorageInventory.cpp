#include "game/StorageInventory.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	StorageInventory::StorageInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, Storage storage_):
		Inventory(std::move(owner), slot_count, active_slot), storage(std::move(storage_)) {}

	StorageInventory::StorageInventory(const StorageInventory &other):
		Inventory(other), storage(other.storage) {}

	StorageInventory::StorageInventory(StorageInventory &&other):
	Inventory(std::forward<Inventory>(other)), storage(std::move(other.storage)) {
		other.weakOwner = {};
		other.slotCount = 0;
		other.activeSlot = 0;
	}

	StorageInventory & StorageInventory::operator=(const StorageInventory &other) {
		weakOwner = other.weakOwner;
		slotCount = other.slotCount.load();
		activeSlot = other.activeSlot.load();
		storage = other.storage;
		return *this;
	}

	StorageInventory & StorageInventory::operator=(StorageInventory &&other) {
		weakOwner = std::move(other.weakOwner);
		slotCount = other.slotCount.exchange(0);
		activeSlot = other.activeSlot.exchange(0);
		{
			auto lock = other.storage.uniqueLock();
			storage = std::move(other.storage);
		}
		return *this;
	}

	ItemStack * StorageInventory::operator[](size_t slot) {
		if (auto iter = storage.find(slot); iter != storage.end())
			return &iter->second;
		return nullptr;
	}

	const ItemStack * StorageInventory::operator[](size_t slot) const {
		if (auto iter = storage.find(slot); iter != storage.end())
			return &iter->second;
		return nullptr;
	}

	void StorageInventory::iterate(const std::function<bool(const ItemStack &, Slot)> &function) {
		for (const auto &[slot, stack]: storage)
			if (function(stack, slot))
				return;
	}

	ItemStack * StorageInventory::firstItem(Slot *slot_out) {
		if (storage.empty()) {
			if (slot_out != nullptr)
				*slot_out = -1;
			return nullptr;
		}

		auto &[slot, stack] = *storage.begin();
		if (slot_out)
			*slot_out = slot;
		return &stack;
	}

	bool StorageInventory::canInsert(const ItemStack &stack) const {
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

	bool StorageInventory::canInsert(const ItemStack &stack, Slot slot) const {
		auto iter = storage.find(slot);

		if (iter == storage.end())
			return stack.count <= stack.item->maxCount;

		const ItemStack &stored = iter->second;

		if (!stored.canMerge(stack))
			return false;

		ssize_t remaining = stack.count;
		const ssize_t storable = ssize_t(stored.item->maxCount) - ssize_t(stored.count);

		if (0 < storable) {
			const ItemCount to_store = std::min<ItemCount>(remaining, storable);
			remaining -= to_store;
			assert(0 <= remaining);
			return remaining == 0;
		}

		return false;
	}

	ItemCount StorageInventory::insertable(const ItemStack &stack, Slot slot) const {
		auto iter = storage.find(slot);

		if (iter == storage.end())
			return stack.count;

		const ItemStack &stored = iter->second;

		if (!stored.canMerge(stack))
			return 0;

		return stored.item->maxCount - stack.count;
	}

	ItemCount StorageInventory::count(const ItemID &id) const {
		if (id.getPath() == "attribute")
			return countAttribute(id);

		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (stored_stack.item->identifier == id)
				out += stored_stack.count;

		return out;
	}

	ItemCount StorageInventory::count(const Item &item) const {
		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (stored_stack.item->identifier == item.identifier)
				out += stored_stack.count;

		return out;
	}

	ItemCount StorageInventory::count(const ItemStack &stack) const {
		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (stack.canMerge(stored_stack))
				out += stored_stack.count;

		return out;
	}

	ItemCount StorageInventory::countAttribute(const Identifier &attribute) const {
		ItemCount out = 0;

		for (const auto &[slot, stack]: storage)
			if (stack.hasAttribute(attribute))
				out += stack.count;

		return out;
	}

	bool StorageInventory::hasSlot(Slot slot) const {
		return 0 <= slot && slot < slotCount;
	}

	bool StorageInventory::empty() const {
		return storage.empty();
	}

	ItemStack & StorageInventory::front() {
		if (storage.empty())
			throw std::out_of_range("Inventory empty");
		return storage.begin()->second;
	}

	const ItemStack & StorageInventory::front() const {
		if (storage.empty())
			throw std::out_of_range("Inventory empty");
		return storage.begin()->second;
	}

	bool StorageInventory::contains(Slot slot) const {
		return storage.contains(slot);
	}

	std::optional<Slot> StorageInventory::find(const ItemID &id) const {
		for (const auto &[slot, stack]: storage)
			if (stack.item->identifier == id)
				return slot;
		return std::nullopt;
	}

	std::optional<Slot> StorageInventory::findAttribute(const Identifier &attribute) const {
		for (const auto &[slot, stack]: storage)
			if (stack.item->attributes.contains(attribute))
				return slot;
		return std::nullopt;
	}

	ItemStack * StorageInventory::getActive() {
		return storage.contains(activeSlot)? &storage.at(activeSlot) : nullptr;
	}

	const ItemStack * StorageInventory::getActive() const {
		return storage.contains(activeSlot)? &storage.at(activeSlot) : nullptr;
	}

	bool StorageInventory::contains(const ItemStack &needle) const {
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

	void StorageInventory::prevSlot() {
		if (0 < activeSlot)
			setActive(activeSlot - 1, false);
	}

	void StorageInventory::nextSlot() {
		if (activeSlot < slotCount - 1)
			setActive(activeSlot + 1, false);
	}

	void StorageInventory::compact() {
		auto lock = storage.uniqueLock();

		for (auto iter = storage.begin(); iter != storage.end();) {
			if (iter->second.count == 0)
				storage.erase(iter++);
			else
				++iter;
		}
	}
}
