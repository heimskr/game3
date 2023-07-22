#pragma once

#include <atomic>
#include <map>
#include <optional>

#include "game/Container.h"
#include "item/Item.h"
#include "recipe/CraftingRequirement.h"
#include "threading/Lockable.h"
#include "util/Castable.h"

namespace Game3 {
	class Agent;
	class Buffer;
	struct CraftingRecipe;

	class Inventory: public Container, public Castable<Inventory> {
		protected:
			Inventory() = default;
			Inventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot = 0);
			Inventory(const Inventory &);
			Inventory(Inventory &&);

		public:
			std::weak_ptr<Agent> weakOwner;
			std::atomic<Slot> slotCount = 0;
			std::atomic<Slot> activeSlot = 0;

			Inventory & operator=(const Inventory &);
			Inventory & operator=(Inventory &&);

			virtual ItemStack * operator[](size_t) = 0;
			virtual const ItemStack * operator[](size_t) const = 0;

			/** If the ItemStack couldn't be inserted into the inventory, this function returns an ItemStack
			 *  containing the leftovers that couldn't be inserted. Otherwise, this function returns nothing. */
			virtual std::optional<ItemStack> add(const ItemStack &, Slot start) = 0;

			virtual std::optional<ItemStack> add(const ItemStack &stack) { return add(stack, -1); }

			virtual bool canInsert(const ItemStack &) const = 0;

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

			virtual bool empty() const = 0;

			/** Counts the number of an item in the inventory. */
			virtual ItemCount count(const ItemID &) const = 0;

			/** Counts the number of an item in the inventory. */
			virtual ItemCount count(const Item &) const = 0;

			/** Counts the number of an item in the inventory.
			 *  This takes ItemStack data into account but ignores the given ItemStack's count. */
			virtual ItemCount count(const ItemStack &) const = 0;

			/** Counts the number of items with a given attribute in the inventory. */
			virtual ItemCount countAttribute(const Identifier &) const = 0;

			std::shared_ptr<Agent> getOwner() const;

			virtual ItemStack & front() = 0;
			virtual const ItemStack & front() const = 0;

			/** Attempts to remove a given amount of an item from the inventory.
			 *  Returns the count removed. */
			virtual ItemCount remove(const ItemStack &) = 0;

			/** Attempts to remove a given amount of an item from a specific slot.
			 *  Returns the count removed. */
			virtual ItemCount remove(const ItemStack &, Slot) = 0;

			virtual ItemCount remove(const CraftingRequirement &) = 0;

			virtual ItemCount remove(const AttributeRequirement &) = 0;

			virtual bool contains(Slot) const = 0;

			/** Returns whether the inventory contains at least a minimum amount of a given item. */
			virtual bool contains(const ItemStack &) const = 0;

			/** Returns the slot containing a given item ID if one exists. */
			virtual std::optional<Slot> find(const ItemID &) const = 0;

			/** Returns the first slot containing an item with the given attribute if one exists. */
			virtual std::optional<Slot> findAttribute(const Identifier &) const = 0;

			virtual ItemStack * getActive() = 0;

			virtual const ItemStack * getActive() const = 0;

			virtual void setActive(Slot, bool force) = 0;

			virtual void setActive(Slot slot) { setActive(slot, false); }

			virtual void prevSlot();

			virtual void nextSlot();

			virtual void notifyOwner() = 0;

			/** Returns the number of times a recipe can be crafted with the inventory's items.
			 *  Doesn't take the output of the recipe into account. */
			virtual ItemCount craftable(const CraftingRecipe &) const = 0;

			static std::shared_ptr<Inventory> create(Side side, std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot = 0, std::map<Slot, ItemStack> storage = {});

		protected:
			/** Removes every slot whose item count is zero from the storage map. */
			virtual void compact() = 0;
	};
}
