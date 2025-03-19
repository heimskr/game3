#pragma once

#include "item/Item.h"

namespace Game3 {
	class FluidGun: public Item {
		public:
			using Item::Item;

			std::string getTooltip(const ConstItemStackPtr &) override;
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool fire(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;

			bool fireGun(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, uint16_t tick_frequency);
	};
}
