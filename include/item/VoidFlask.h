#pragma once

#include "item/Item.h"

namespace Game3 {
	class VoidFlask: public Item {
		public:
			using Item::Item;
			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<double, double>) override;
	};
}
