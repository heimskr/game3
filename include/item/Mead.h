#pragma once

#include "interface/HasFluidType.h"
#include "item/Food.h"

namespace Game3 {
	class Mead: public Food, public HasFluidType {
		public:
			using Food::Food;

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			HitPoints getHealedPoints(const std::shared_ptr<Player> &) override;

			using Food::use;

			Identifier getFluidType() const override { return "base:fluid/mead"; }
	};
}
