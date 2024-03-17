#include "game/InventoryWrapper.h"

namespace Game3 {
	InventoryWrapper::InventoryWrapper(std::shared_ptr<Inventory> inventory_):
	inventory(std::move(inventory_)) {
		weakOwner = inventory->weakOwner;
		// TODO!: getter method for active slot
		activeSlot = inventory->activeSlot.load();
	}

	Slot InventoryWrapper::getSlotCount() const {
		return inventory->getSlotCount();
	}

	void InventoryWrapper::setSlotCount(Slot new_count) {
		inventory->setSlotCount(new_count);
	}

	void InventoryWrapper::set(Slot slot, ItemStackPtr stack) {
		inventory->set(slot, std::move(stack));
	}

	std::unique_ptr<Inventory> InventoryWrapper::copy() const {
		return std::make_unique<InventoryWrapper>(inventory->copy());
	}

	ItemStackPtr InventoryWrapper::operator[](Slot slot) const {
		return validateSlot(slot)? (*inventory)[slot] : nullptr;
	}

	void InventoryWrapper::iterate(const Predicate &iterator) const {
		inventory->iterate([&](const ItemStackPtr &stack, Slot slot) {
			return validateSlot(slot) && iterator(stack, slot);
		});
	}

	ItemStackPtr InventoryWrapper::firstItem(Slot *slot_out) {
		return inventory->firstItem(slot_out, [&](const ItemStackPtr &, Slot slot) {
			return validateSlot(slot);
		});
	}

	ItemStackPtr InventoryWrapper::firstItem(Slot *slot_out, const Predicate &predicate) {
		return inventory->firstItem(slot_out, [&](const ItemStackPtr &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	ItemStackPtr InventoryWrapper::add(const ItemStackPtr &stack, const SlotPredicate &predicate, Slot start) {
		return inventory->add(stack, [&](Slot slot) {
			return validateSlot(slot) && predicate(slot);
		}, adjustSlot(start));
	}

	bool InventoryWrapper::canInsert(const ItemStackPtr &stack, const SlotPredicate &predicate) const {
		return inventory->canInsert(stack, [&](Slot slot) {
			return validateSlot(slot) && predicate(slot);
		});
	}

	bool InventoryWrapper::canInsert(const ItemStackPtr &stack, Slot slot) const {
		return validateSlot(slot) && inventory->canInsert(stack, slot);
	}

	bool InventoryWrapper::canInsert(const ItemStackPtr &stack) const {
		return inventory->canInsert(stack, [&](Slot slot) {
			return validateSlot(slot);
		});
	}

	bool InventoryWrapper::canExtract(Slot slot) const {
		return validateSlot(slot) && inventory->canExtract(slot);
	}

	ItemCount InventoryWrapper::insertable(const ItemStackPtr &stack, Slot slot) const {
		return validateSlot(slot)? inventory->insertable(stack, slot) : 0;
	}

	void InventoryWrapper::drop(Slot slot) {
		if (validateSlot(slot))
			inventory->drop(slot);
	}

	void InventoryWrapper::discard(Slot slot) {
		if (validateSlot(slot))
			inventory->discard(slot);
	}

	void InventoryWrapper::swap(Slot source, Slot destination) {
		if (validateSlot(source) && validateSlot(destination))
			inventory->swap(source, destination);
	}

	void InventoryWrapper::erase(Slot slot) {
		if (validateSlot(slot))
			inventory->erase(slot);
	}

	void InventoryWrapper::clear() {
		for (Slot slot = 0, slot_max = inventory->getSlotCount(); slot < slot_max; ++slot) {
			if (validateSlot(slot))
				inventory->erase(slot);
		}
	}

	ItemCount InventoryWrapper::count(const ItemID &id) const {
		if (id.getPathStart() == "attribute")
			return countAttribute(id);

		ItemCount out = 0;

		iterate([&](const ItemStackPtr &stack, Slot) {
			if (stack->item->identifier == id)
				out += stack->count;
			return false;
		});

		return out;
	}

	ItemCount InventoryWrapper::count(const Item &item) const {
		ItemCount out = 0;

		iterate([&](const ItemStackPtr &stack, Slot) {
			if (stack->item->identifier == item.identifier)
				out += stack->count;
			return false;
		});

		return out;
	}

	ItemCount InventoryWrapper::count(const ItemStackPtr &stack) const {
		ItemCount out = 0;

		iterate([&](const ItemStackPtr &stored_stack, Slot) {
			if (stack->canMerge(*stored_stack))
				out += stored_stack->count;
			return false;
		});

		return out;
	}

	ItemCount InventoryWrapper::count(const ItemStackPtr &stack, const SlotPredicate &predicate) const {
		return inventory->count(stack, [&](Slot slot) {
			return validateSlot(slot) && predicate(slot);
		});
	}

	ItemCount InventoryWrapper::countAttribute(const Identifier &attribute) const {
		ItemCount out = 0;

		iterate([&](const ItemStackPtr &stack, Slot) {
			if (stack->hasAttribute(attribute))
				out += stack->count;
			return false;
		});

		return out;
	}

	bool InventoryWrapper::hasSlot(Slot slot) const {
		return validateSlot(slot) && inventory->hasSlot(slot);
	}

	ItemStackPtr InventoryWrapper::front() const {
		ItemStackPtr out;

		iterate([&](const ItemStackPtr &stack, Slot) {
			out = stack;
			return true;
		});

		if (!out)
			throw std::out_of_range("InventoryWrapper empty");

		return out;
	}

	ItemCount InventoryWrapper::remove(const ItemStackPtr &stack) {
		return inventory->remove(stack, [this](const ItemStackPtr &, Slot slot) {
			return validateSlot(slot);
		});
	}

	ItemCount InventoryWrapper::remove(const ItemStackPtr &stack, const Predicate &predicate) {
		return inventory->remove(stack, [&](const ItemStackPtr &candidate, Slot slot) {
			return validateSlot(slot) && predicate(candidate, slot);
		});
	}

	ItemCount InventoryWrapper::remove(const ItemStackPtr &stack, Slot slot) {
		return validateSlot(slot)? inventory->remove(stack, slot) : 0;
	}

	ItemCount InventoryWrapper::remove(const CraftingRequirement &requirement, const Predicate &predicate) {
		return inventory->remove(requirement, [&](const ItemStackPtr &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	ItemCount InventoryWrapper::remove(const AttributeRequirement &requirement, const Predicate &predicate) {
		return inventory->remove(requirement, [&](const ItemStackPtr &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	bool InventoryWrapper::contains(Slot slot) const {
		return validateSlot(slot) && inventory->contains(slot);
	}

	bool InventoryWrapper::contains(const ItemStackPtr &stack) const {
		return inventory->contains(stack, [this](const ItemStackPtr &, Slot slot) {
			return validateSlot(slot);
		});
	}

	bool InventoryWrapper::contains(const ItemStackPtr &stack, const Predicate &predicate) const {
		return inventory->contains(stack, [this, &predicate](const ItemStackPtr &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	std::optional<Slot> InventoryWrapper::find(const ItemID &id) const {
		return inventory->find(id, [this](const ItemStackPtr &, Slot slot) {
			return validateSlot(slot);
		});
	}

	std::optional<Slot> InventoryWrapper::find(const ItemID &id, const Predicate &predicate) const {
		return inventory->find(id, [&](const ItemStackPtr &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	std::optional<Slot> InventoryWrapper::findAttribute(const Identifier &attribute) const {
		return inventory->find(attribute, [this](const ItemStackPtr &, Slot slot) {
			return validateSlot(slot);
		});
	}

	std::optional<Slot> InventoryWrapper::findAttribute(const Identifier &attribute, const Predicate &predicate) const {
		return inventory->findAttribute(attribute, [&](const ItemStackPtr &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	ItemStackPtr InventoryWrapper::getActive() const {
		return inventory->getActive();
	}

	void InventoryWrapper::setActive(Slot slot, bool force) {
		if (validateSlot(slot))
			inventory->setActive(slot, force);
	}

	void InventoryWrapper::notifyOwner() {
		inventory->notifyOwner();
	}

	bool InventoryWrapper::empty() const {
		return inventory->empty();
	}

	Slot InventoryWrapper::slotsOccupied() const {
		Slot out = 0;

		iterate([this, &out](const ItemStackPtr &stack, Slot slot) {
			if (stack && validateSlot(slot))
				++out;
			return false;
		});

		return out;
	}

	void InventoryWrapper::replace(const Inventory &other) {
		inventory->replace(other);
	}

	void InventoryWrapper::replace(Inventory &&other) {
		inventory->replace(std::move(other));
	}

	std::unique_lock<DefaultMutex> InventoryWrapper::uniqueLock() const {
		return inventory->uniqueLock();
	}

	std::shared_lock<DefaultMutex> InventoryWrapper::sharedLock() const {
		return inventory->sharedLock();
	}

	void InventoryWrapper::compact() {
		inventory->compact();
	}
}
