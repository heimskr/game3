#pragma once

#include "item/Item.h"

namespace Game3 {
	class Wrench: public Item {
		public:
			Wrench(ItemID id_, std::string name_, MoneyCount base_price);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
