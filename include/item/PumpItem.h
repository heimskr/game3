#pragma once

#include "item/Item.h"

namespace Game3 {
	class PumpItem: public Item {
		public:
			using Item::Item;
			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
