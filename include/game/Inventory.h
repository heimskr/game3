#pragma once

#include <map>
#include <optional>

#include "game/Container.h"
#include "item/Item.h"
#include "recipe/CraftingRequirement.h"

namespace Game3 {
	class Buffer;
	struct Agent;
	struct CraftingRecipe;

	class Inventory: public Container {
		public:
			std::weak_ptr<Agent> owner;
			Slot slotCount = 0;
			Slot activeSlot = 0;

			Inventory() = default;
			Inventory(const std::shared_ptr<Agent> &owner_, Slot slot_count);

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

			/** Counts the number of an item in the inventory. */
			ItemCount count(const ItemID &) const;

			/** Counts the number of an item in the inventory. */
			ItemCount count(const Item &) const;

			/** Counts the number of an item in the inventory. This takes ItemStack data into account but ignores the given ItemStack's count. */
			ItemCount count(const ItemStack &) const;

			/** Counts the number of items with a given attribute in the inventory. */
			ItemCount countAttribute(const Identifier &) const;

			std::shared_ptr<Agent> getOwner() const;

			ItemStack & front();
			const ItemStack & front() const;

			/** Attempts to remove a given amount of an item from the inventory. Returns the count removed. */
			ItemCount remove(const ItemStack &);

			/** Attempts to remove a given amount of an item from a specific slot. Returns the count removed. */
			ItemCount remove(const ItemStack &, Slot);

			ItemCount remove(const CraftingRequirement &);

			ItemCount remove(const AttributeRequirement &);

			bool contains(Slot) const;

			/** Returns whether the inventory contains at least a minimum amount of a given item. */
			bool contains(const ItemStack &) const;

			/** Returns the slot containing a given item ID if one exists. */
			std::optional<Slot> find(const ItemID &) const;

			/** Returns the first slot containing an item with the given attribute if one exists. */
			std::optional<Slot> findAttribute(const Identifier &) const;

			ItemStack * getActive();

			const ItemStack * getActive() const;

			void setActive(Slot);

			void prevSlot();

			void nextSlot();

			void notifyOwner();

			/** Returns the number of times a recipe can be crafted with the inventory's items. Doesn't take the output of the recipe into account. */
			ItemCount craftable(const CraftingRecipe &) const;

		private:
			std::map<Slot, ItemStack> storage;

			/** Removes every slot whose item count is zero from the storage map. */
			void compact();

		public:
			inline const decltype(storage) & getStorage() const { return storage; }
			inline void setStorage(decltype(storage) new_storage) { storage = std::move(new_storage); }
			inline Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &game, Slot slot) { return storage.at(slot).getImage(game); }

			static Inventory fromJSON(Game &, const nlohmann::json &, const std::shared_ptr<Agent> &);

			friend void to_json(nlohmann::json &, const Inventory &);
	};


	template <typename T>
	T popBuffer(Buffer &);
	template <>
	Inventory popBuffer<Inventory>(Buffer &);
	Buffer & operator+=(Buffer &, const Inventory &);
	Buffer & operator<<(Buffer &, const Inventory &);
	Buffer & operator>>(Buffer &, Inventory &);

	void to_json(nlohmann::json &, const Inventory &);
}
