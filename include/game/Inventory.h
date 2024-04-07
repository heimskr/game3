#pragma once

#include "game/Container.h"
#include "item/Item.h"
#include "recipe/CraftingRequirement.h"
#include "threading/Atomic.h"
#include "threading/HasMutex.h"
#include "threading/Lockable.h"

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <optional>

namespace Game3 {
	class Agent;
	struct CraftingRecipe;

	/** Inventories should be locked appropriately (see HasMutex) when something is calling Inventory methods. The Inventory will not lock itself. */
	class Inventory: public Container, public HasMutex<> {
		protected:
			Inventory();
			Inventory(std::shared_ptr<Agent> owner, Slot active_slot = 0, InventoryID index_ = 0);

		public:
			using SlotPredicate = std::function<bool(Slot)>;
			using Predicate = std::function<bool(const ItemStackPtr &, Slot)>;

			std::weak_ptr<Agent> weakOwner;
			Atomic<Slot> activeSlot = 0;
			Atomic<InventoryID> index = -1;
			/** Called before the swap occurs. The first argument always refers to this object.
			 *  Returns (optionally) a function to call after the swap has occurred. */
			std::function<std::function<void()>(Inventory &, Slot, Inventory &, Slot)> onSwap;
			/** Called before the move occurs. The first pair of arguments are always the source and may or not refer to this object;
			 *  the second pair of arguments are always the destination. Returns (optionally) a function to call after the move has occurred. */
			std::function<std::function<void()>(Inventory &, Slot from, Inventory &, Slot to, bool consumed)> onMove;
			/** Called before the removal occurs. Returns (optionally) a function to call after the removal has occurred. */
			std::function<std::function<void()>(Slot)> onRemove;

			Inventory & operator=(const Inventory &);
			Inventory & operator=(Inventory &&);

			virtual std::unique_ptr<Inventory> copy() const = 0;

			virtual ItemStackPtr operator[](Slot) const = 0;

			bool operator==(const Inventory &other) const;

			virtual void set(Slot, ItemStackPtr) = 0;

			virtual Slot getSlotCount() const = 0;
			virtual void setSlotCount(Slot) = 0;

			/** Iterates over all items in the inventory until all have been iterated or the iteration function returns true. */
			virtual void iterate(const Predicate &) const = 0;

			virtual ItemStackPtr firstItem(Slot *slot_out) = 0;
			virtual ItemStackPtr firstItem(Slot *slot_out, const Predicate &) = 0;

			/** If the ItemStack couldn't be inserted into the inventory, this function returns an ItemStack
			 *  containing the leftovers that couldn't be inserted. Otherwise, this function returns a null pointer.
			 *  The optional predicate will be used to determine which slots can be inserted into. */
			virtual ItemStackPtr add(const ItemStackPtr &stack) { return add(stack, [](Slot) { return true; }, -1); }
			virtual ItemStackPtr add(const ItemStackPtr &, const SlotPredicate &predicate, Slot start) = 0;
			virtual ItemStackPtr add(const ItemStackPtr &stack, const SlotPredicate &predicate) { return add(stack, predicate, -1); }
			virtual ItemStackPtr add(const ItemStackPtr &stack, Slot start) { return add(stack, [](Slot) { return true; }, start); }

			virtual bool canInsert(const ItemStackPtr &, const SlotPredicate &predicate) const = 0;
			virtual bool canInsert(const ItemStackPtr &, Slot) const = 0;
			virtual bool canInsert(const ItemStackPtr &stack) const { return canInsert(stack, [](Slot) { return true; }); }

			virtual bool canExtract(Slot) const = 0;

			virtual ItemCount insertable(const ItemStackPtr  &, Slot) const = 0;

			/** Does notify the owner. */
			virtual bool decrease(const ItemStackPtr &, Slot, ItemCount amount, bool do_lock);

			/** Removes an item from the inventory and drops it at the owner's location. */
			virtual void drop(Slot) = 0;

			/** Like erase, but sends a packet to the server instead if run on a client. */
			virtual void discard(Slot) = 0;

			/** Swaps two slots. */
			virtual void swap(Slot, Slot) = 0;

			/** Erases a given slot. Doesn't notify the owner! */
			virtual void erase(Slot) = 0;

			/** Erases the active slot. Doesn't notify the owner! */
			virtual void erase();

			virtual void clear() = 0;

			virtual bool empty() const = 0;

			/** Counts the number of an item in the inventory. */
			virtual ItemCount count(const ItemID &) const = 0;

			/** Counts the number of an item in the inventory. */
			virtual ItemCount count(const Item &) const = 0;

			/** Counts the number of an item in the inventory.
			 *  This takes ItemStack data into account but ignores the given ItemStack's count. */
			virtual ItemCount count(const ItemStackPtr &) const = 0;

			/** Counts the number of an item in the inventory, given a predicate to select the slots read from.
			 *  This takes ItemStack data into account but ignores the given ItemStack's count. */
			virtual ItemCount count(const ItemStackPtr &, const SlotPredicate &) const = 0;

			/** Counts the number of items with a given attribute in the inventory. */
			virtual ItemCount countAttribute(const Identifier &) const = 0;

			virtual bool hasSlot(Slot) const = 0;

			std::shared_ptr<Agent> getOwner() const;

			virtual ItemStackPtr front() const = 0;

			/** Attempts to remove a given amount of an item from the inventory.
			 *  Returns the count removed. */
			virtual ItemCount remove(const ItemStackPtr &) = 0;

			/** Attempts to remove a given amount of an item from the inventory.
			 *  Uses a predicate to determine which slots can be removed from.
			 *  Returns the count removed. */
			virtual ItemCount remove(const ItemStackPtr &, const Predicate &) = 0;

			/** Attempts to remove a given amount of an item from a specific slot. Returns the count removed. */
			virtual ItemCount remove(const ItemStackPtr &, Slot) = 0;

			virtual ItemCount remove(const CraftingRequirement &, const Predicate &) = 0;
			virtual ItemCount remove(const CraftingRequirement &requirement) { return remove(requirement, [](const ItemStackPtr &, Slot) { return true; }); }

			virtual ItemCount remove(const AttributeRequirement &, const Predicate &) = 0;
			virtual ItemCount remove(const AttributeRequirement &requirement) { return remove(requirement, [](const ItemStackPtr &, Slot) { return true; }); }

			virtual bool contains(Slot) const = 0;

			/** Returns whether the inventory contains at least a minimum amount of a given item. */
			virtual bool contains(const ItemStackPtr &stack) const { return contains(stack, [](const ItemStackPtr &, Slot) { return true; }); }

			/** Returns whether the inventory contains at least a minimum amount of a given item, given a predicate. */
			virtual bool contains(const ItemStackPtr &, const Predicate &) const = 0;

			/** Returns the slot containing a given item ID if one exists. */
			virtual std::optional<Slot> find(const ItemID &id) const { return find(id, [](const ItemStackPtr &, Slot) { return true; }); }
			/** Returns the slot containing a given item ID if one exists and matches a predicate. */
			virtual std::optional<Slot> find(const ItemID &, const Predicate &) const = 0;

			/** Returns the first slot containing an item with the given attribute if one exists. */
			virtual std::optional<Slot> findAttribute(const Identifier &attribute) const { return findAttribute(attribute, [](const ItemStackPtr &, Slot) { return true; }); }
			/** Returns the first slot containing an item with the given attribute if one exists and matches a predicate. */
			virtual std::optional<Slot> findAttribute(const Identifier &, const Predicate &) const = 0;

			virtual ItemStackPtr getActive() const = 0;

			virtual void setActive(Slot, bool force) = 0;
			virtual void setActive(Slot slot) { setActive(slot, false); }

			virtual void prevSlot();
			virtual void nextSlot();

			virtual void notifyOwner() = 0;

			/** Returns the number of times a recipe can be crafted with the inventory's items.
			 *  Doesn't take the output of the recipe into account. */
			virtual ItemCount craftable(const CraftingRecipe &) const;

			virtual Slot slotsOccupied() const = 0;

			virtual void replace(const Inventory &) = 0;
			virtual void replace(Inventory &&) = 0;

			static std::shared_ptr<Inventory> create(Side side, std::shared_ptr<Agent> owner, Slot slot_count, InventoryID index = 0, Slot active_slot = 0, std::map<Slot, ItemStackPtr> storage = {});
			static std::shared_ptr<Inventory> create(std::shared_ptr<Agent> owner, Slot slot_count, InventoryID index = 0, Slot active_slot = 0, std::map<Slot, ItemStackPtr> storage = {});

		protected:
			/** Removes every slot whose item count is zero from the storage map. */
			virtual void compact() = 0;

			Atomic<bool> suppressInventoryNotifications{false};

		public:
			struct Suppressor {
				Inventory &parent;
				bool active = true;

				explicit Suppressor(Inventory &parent_): parent(parent_) {
					parent.suppressInventoryNotifications = true;
				}

				~Suppressor() {
					if (active)
						parent.suppressInventoryNotifications = false;
				}

				void cancel(bool notify = false) {
					active = false;
					parent.suppressInventoryNotifications = false;
					if (notify)
						parent.notifyOwner();
				}
			};

			Suppressor suppress() {
				return Suppressor(*this);
			}

		friend class InventoryWrapper;
	};

	using InventoryPtr = std::shared_ptr<Inventory>;
}
