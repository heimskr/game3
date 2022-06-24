#pragma once

#include <map>
#include <optional>

#include "item/Item.h"

namespace Game3 {
	struct CraftingRecipe;
	struct HasRealm;

	class Inventory {
		public:
			std::weak_ptr<HasRealm> owner;
			Slot slotCount = 0;
			Slot activeSlot = 0;

			Inventory(const std::shared_ptr<HasRealm> &owner_, Slot slot_count);

			ItemStack * operator[](size_t);
			const ItemStack * operator[](size_t) const;

			/** If the ItemStack couldn't be inserted into the inventory, this function returns an ItemStack containing the leftovers that couldn't be inserted.
			 *  Otherwise, this function returns nothing. */
			std::optional<ItemStack> add(const ItemStack &, Slot start = -1);

			bool canStore(const ItemStack &) const;

			/** Removes an item from the inventory and drops it at the owner's location. */
			void drop(Slot);

			/** Swaps two slots. Returns true if at least one of the first slot contained an item and the second slot was valid. */
			bool swap(Slot, Slot);

			void erase(Slot, bool suppress_notification = false);

			/** Erases the active slot. */
			void erase(bool suppress_notification = false);

			inline bool empty() const { return storage.empty(); }

			/** Counts the amount of an item in the inventory. */
			ItemCount count(ItemID) const;

			/** Counts the amount of an item in the inventory. */
			ItemCount count(const Item &) const;

			/** Counts the amount of an item in the inventory. This takes ItemStack data into account but ignores the given ItemStack's count. */
			ItemCount count(const ItemStack &) const;

			std::shared_ptr<HasRealm> getOwner() const;

			ItemStack & front();
			const ItemStack & front() const;

			/** Attempts to remove a given amount of an item from the inventory. Returns the count removed. */
			ItemCount remove(const ItemStack &);

			/** Attempts to remove a given amount of an item from a specific slot. Returns the count removed. */
			ItemCount remove(const ItemStack &, Slot);

			bool contains(Slot) const;

			/** Returns whether the inventory contains at least a minimum amount of a given item. */
			bool contains(const ItemStack &) const;

			/** Returns the slot containing a given item ID if one exists. */
			std::optional<Slot> find(ItemID) const;

			/** Returns the first slot containing an item with the given attribute if one exists. */
			std::optional<Slot> find(ItemAttribute) const;

			ItemStack * getActive();

			const ItemStack * getActive() const;

			void setActive(Slot);

			void prevSlot();

			void nextSlot();

			void notifyOwner();

			/** Returns the number of times a recipe can be crafted with the inventory's items. Doesn't take the output of the recipe into account. */
			ItemCount craftable(const CraftingRecipe &) const;

			bool canCraft(const CraftingRecipe &) const;

			/** Crafts a recipe. Returns whether the recipe could be crafted. */
			bool craft(const CraftingRecipe &, std::vector<ItemStack> &leftovers);

		private:
			std::map<Slot, ItemStack> storage;

		public:
			inline const decltype(storage) & getStorage() const { return storage; }
			inline Glib::RefPtr<Gdk::Pixbuf> getImage(Slot slot) { return storage.at(slot).getImage(); }

			static Inventory fromJSON(const nlohmann::json &, const std::shared_ptr<HasRealm> &);

			friend void to_json(nlohmann::json &, const Inventory &);
	};

	void to_json(nlohmann::json &, const Inventory &);
}
