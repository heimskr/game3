#pragma once

#include <map>

#include "game/Item.h"

namespace Game3 {
	class Inventory {
		public:
			Slot slotCount = 0;

			Inventory() = default;
			Inventory(Slot slot_count): slotCount(slot_count) {}

			ItemStack * operator[](size_t);
			const ItemStack * operator[](size_t) const;

			/** If the ItemStack couldn't be inserted into the inventory, this function returns an ItemStack containing the leftovers that couldn't be inserted.
			 *  Otherwise, this function returns a null pointer. */
			std::unique_ptr<ItemStack> add(const ItemStack &);

			inline bool empty() const { return storage.empty(); }

		private:
			std::map<Slot, ItemStack> storage;

		public:
			inline const decltype(storage) & getStorage() const { return storage; }

			friend void to_json(nlohmann::json &, const Inventory &);
			friend void from_json(const nlohmann::json &, Inventory &);
	};

	void to_json(nlohmann::json &, const Inventory &);
	void from_json(const nlohmann::json &, Inventory &);
}
