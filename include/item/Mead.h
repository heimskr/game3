#pragma once

#include "item/Food.h"

namespace Game3 {
	class Mead: public Food {
		public:
			using Food::Food;

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<double, double>) override;
			HitPoints getHealedPoints(const std::shared_ptr<Player> &) override;

			using Food::use;
	};
}
