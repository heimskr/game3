#pragma once

#include "item/Tool.h"

namespace Game3 {
	class Hoe: public Tool {
		public:
			Hoe(ItemID id_, std::string name_, MoneyCount base_price, Durability max_durability);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
