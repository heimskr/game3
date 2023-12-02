#pragma once

#include "item/Tool.h"

namespace Game3 {
	class Hoe: public Tool {
		public:
			Hoe(ItemID id_, std::string name_, MoneyCount base_price, Durability max_durability);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
	};
}
