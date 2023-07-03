#pragma once

#include "item/Item.h"

namespace Game3 {
	class ItemPipe: public Item {
		public:
			ItemPipe(MoneyCount base_price);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
