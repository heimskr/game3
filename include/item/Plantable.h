#pragma once

#include "item/Item.h"
#include "types/Layer.h"

namespace Game3 {
	class Plantable: public Item {
		public:
			template <typename... Args>
			Plantable(Args &&...args): Item(std::forward<Args>(args)...) {
				addAttribute("base:attribute/plantable");
			}

			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float> offsets, DragAction) override;

			/** Implementations should assume the inventory is already locked properly and not lock it themselves. */
			virtual bool plant(InventoryPtr, Slot, const ItemStackPtr &, const Place &, Layer) = 0;
	};
}
