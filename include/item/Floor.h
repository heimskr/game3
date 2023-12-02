#pragma once

#include "item/Item.h"

namespace Game3 {
	class Floor: public Item {
		public:
			Identifier tilename;
			Floor(ItemID, std::string name_, Identifier tilename_, MoneyCount base_price, ItemCount max_count = 64);
			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
	};
}
