#pragma once

#include <map>
#include <memory>

#include "game/Item.h"

namespace Game3 {
	class Entity;

	class Inventory {
		public:
			std::weak_ptr<Entity> owner;
			Slot slotCount = 0;

			Inventory(const std::shared_ptr<Entity> &owner_, Slot slot_count);

			ItemStack * operator[](size_t);
			const ItemStack * operator[](size_t) const;

			/** If the ItemStack couldn't be inserted into the inventory, this function returns an ItemStack containing the leftovers that couldn't be inserted.
			 *  Otherwise, this function returns a null pointer. */
			std::unique_ptr<ItemStack> add(const ItemStack &);

			/** Removes an item from the inventory and drops it at the owner's location. */
			void drop(Slot);

			/** Swaps two slots. Returns true if at least one of the first slot contained an item and the second slot was valid. */
			bool swap(Slot, Slot);

			inline bool empty() const { return storage.empty(); }

		private:
			std::map<Slot, ItemStack> storage;

		public:
			inline const decltype(storage) & getStorage() const { return storage; }

			static Inventory fromJSON(const nlohmann::json &, const std::shared_ptr<Entity> &);

			friend void to_json(nlohmann::json &, const Inventory &);
	};

	void to_json(nlohmann::json &, const Inventory &);
}
