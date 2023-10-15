#include "game/InventoryWrapper.h"

namespace Game3 {
	InventoryWrapper::InventoryWrapper(std::shared_ptr<Inventory> inventory_):
	inventory(std::move(inventory_)) {
		weakOwner = inventory->weakOwner;
		slotCount = inventory->slotCount.load();
		// TODO!: getter method for active slot
		activeSlot = inventory->activeSlot.load();
	}

	std::unique_ptr<Inventory> InventoryWrapper::copy() const {
		return std::make_unique<InventoryWrapper>(inventory->copy());
	}

	ItemStack * InventoryWrapper::operator[](Slot slot) {
		return validateSlot(slot)? (*inventory)[slot] : nullptr;
	}

	const ItemStack * InventoryWrapper::operator[](Slot slot) const {
		return validateSlot(slot)? (*inventory)[slot] : nullptr;
	}

	void InventoryWrapper::iterate(const std::function<bool(const ItemStack &, Slot)> &iterator) const {
		inventory->iterate([&](const ItemStack &stack, Slot slot) {
			return validateSlot(slot) && iterator(stack, slot);
		});
	}

	void InventoryWrapper::iterate(const std::function<bool(ItemStack &, Slot)> &iterator) {
		inventory->iterate([&](ItemStack &stack, Slot slot) {
			return validateSlot(slot) && iterator(stack, slot);
		});
	}

	ItemStack * InventoryWrapper::firstItem(Slot *slot_out) {
		return inventory->firstItem(slot_out, [&](const ItemStack &, Slot slot) {
			return validateSlot(slot);
		});
	}

	ItemStack * InventoryWrapper::firstItem(Slot *slot_out, const std::function<bool(const ItemStack &, Slot)> &predicate) {
		return inventory->firstItem(slot_out, [&](const ItemStack &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	std::optional<ItemStack> InventoryWrapper::add(const ItemStack &stack, const std::function<bool(Slot)> &predicate, Slot start) {
		return inventory->add(stack, [&](Slot slot) {
			return validateSlot(slot) && predicate(slot);
		}, adjustSlot(start));
	}

	bool InventoryWrapper::canInsert(const ItemStack &stack, const std::function<bool(Slot)> &predicate) const {
		return inventory->canInsert(stack, [&](Slot slot) {
			return validateSlot(slot) && predicate(slot);
		});
	}

	bool InventoryWrapper::canInsert(const ItemStack &stack, Slot slot) const {
		return validateSlot(slot) && inventory->canInsert(stack, slot);
	}

	bool InventoryWrapper::canInsert(const ItemStack &stack) const {
		return inventory->canInsert(stack, [&](Slot slot) {
			return validateSlot(slot);
		});
	}

	bool InventoryWrapper::canExtract(Slot slot) const {
		return validateSlot(slot) && inventory->canExtract(slot);
	}

	ItemCount InventoryWrapper::insertable(const ItemStack &stack, Slot slot) const {
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

	ItemCount InventoryWrapper::count(const ItemID &id) const {
		if (id.getPathStart() == "attribute")
			return countAttribute(id);

		ItemCount out = 0;

		iterate([&](const ItemStack &stack, Slot) {
			if (stack.item->identifier == id)
				out += stack.count;
			return false;
		});

		return out;
	}

	ItemCount InventoryWrapper::count(const Item &item) const {
		ItemCount out = 0;

		iterate([&](const ItemStack &stack, Slot) {
			if (stack.item->identifier == item.identifier)
				out += stack.count;
			return false;
		});

		return out;
	}

	ItemCount InventoryWrapper::count(const ItemStack &stack) const {
		ItemCount out = 0;

		iterate([&](const ItemStack &stored_stack, Slot) {
			if (stack.canMerge(stored_stack))
				out += stored_stack.count;
			return false;
		});

		return out;
	}

	ItemCount InventoryWrapper::count(const ItemStack &stack, const std::function<bool(Slot)> &predicate) const {
		return inventory->count(stack, [&](Slot slot) {
			return validateSlot(slot) && predicate(slot);
		});
	}

	ItemCount InventoryWrapper::countAttribute(const Identifier &attribute) const {
		ItemCount out = 0;

		iterate([&](const ItemStack &stack, Slot) {
			if (stack.hasAttribute(attribute))
				out += stack.count;
			return false;
		});

		return out;
	}

	bool InventoryWrapper::hasSlot(Slot slot) const {
		return validateSlot(slot) && inventory->hasSlot(slot);
	}

	ItemStack & InventoryWrapper::front() {
		ItemStack *out = nullptr;

		iterate([&](ItemStack &stack, Slot) {
			out = &stack;
			return true;
		});

		if (out == nullptr)
			throw std::out_of_range("InventoryWrapper empty");

		return *out;
	}

	const ItemStack & InventoryWrapper::front() const {
		const ItemStack *out = nullptr;

		iterate([&](const ItemStack &stack, Slot) {
			out = &stack;
			return true;
		});

		if (out == nullptr)
			throw std::out_of_range("InventoryWrapper empty");

		return *out;
	}

	ItemCount InventoryWrapper::remove(const ItemStack &stack) {
		return inventory->remove(stack, [this](Slot slot) {
			return validateSlot(slot);
		});
	}

	ItemCount InventoryWrapper::remove(const ItemStack &stack, const std::function<bool(Slot)> &predicate) {
		return inventory->remove(stack, [&](Slot slot) {
			return validateSlot(slot) && predicate(slot);
		});
	}

	ItemCount InventoryWrapper::remove(const ItemStack &stack, Slot slot) {
		return validateSlot(slot)? inventory->remove(stack, slot) : 0;
	}

	ItemCount InventoryWrapper::remove(const CraftingRequirement &requirement, const ConstPredicate &predicate) {
		return inventory->remove(requirement, [&](const ItemStack &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	ItemCount InventoryWrapper::remove(const AttributeRequirement &requirement, const ConstPredicate &predicate) {
		return inventory->remove(requirement, [&](const ItemStack &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	bool InventoryWrapper::contains(Slot slot) const {
		return validateSlot(slot) && inventory->contains(slot);
	}

	bool InventoryWrapper::contains(const ItemStack &stack) const {
		return inventory->contains(stack, [this](const ItemStack &, Slot slot) {
			return validateSlot(slot);
		});
	}

	bool InventoryWrapper::contains(const ItemStack &stack, const ConstPredicate &predicate) const {
		return inventory->contains(stack, [this, &predicate](const ItemStack &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	std::optional<Slot> InventoryWrapper::find(const ItemID &id) const {
		return inventory->find(id, [this](const ItemStack &, Slot slot) {
			return validateSlot(slot);
		});
	}

	std::optional<Slot> InventoryWrapper::find(const ItemID &id, const ConstPredicate &predicate) const {
		return inventory->find(id, [&](const ItemStack &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	std::optional<Slot> InventoryWrapper::findAttribute(const Identifier &attribute) const {
		return inventory->find(attribute, [this](const ItemStack &, Slot slot) {
			return validateSlot(slot);
		});
	}

	std::optional<Slot> InventoryWrapper::findAttribute(const Identifier &attribute, const ConstPredicate &predicate) const {
		return inventory->findAttribute(attribute, [&](const ItemStack &stack, Slot slot) {
			return validateSlot(slot) && predicate(stack, slot);
		});
	}

	ItemStack * InventoryWrapper::getActive() {
		return inventory->getActive();
	}

	const ItemStack * InventoryWrapper::getActive() const {
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

	void InventoryWrapper::compact() {
		inventory->compact();
	}
}
