#pragma once

#include "item/Item.h"

namespace Game3 {
	class Plantable: public Item {
		public:
			template <typename... Args>
			Plantable(Args &&...args): Item(std::forward<Args>(args)...) {
				addAttribute("base:attribute/plantable");
			}

			/** Implementations should assume the inventory is already locked properly and not lock it themselves. */
			virtual bool plant(InventoryPtr, Slot, ItemStack &, const Place &) = 0;
	};
}
