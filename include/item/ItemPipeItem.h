#pragma once

#include "item/Item.h"

namespace Game3 {
	class ItemPipeItem: public Item {
		public:
			ItemPipeItem(MoneyCount base_price);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
