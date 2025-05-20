#pragma once

#include "item/Item.h"

namespace Game3 {
	class FluidGun: public Item {
		public:
			using Item::Item;

			std::string getTooltip(const ConstItemStackPtr &) override;
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;
			bool fire(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			void renderEffects(Window &, const RendererContext &, const Position &, Modifiers, const ItemStackPtr &) const override;

			bool fireGun(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, uint16_t tick_frequency);
	};
}
