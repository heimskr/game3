#pragma once

#include "item/Food.h"

namespace Game3 {
	class BasicFood: public Food {
		public:
			BasicFood(ItemID id_, std::string name_, MoneyCount base_price, HitPoints healed_points = 1, ItemCount max_count = 64):
				Food(std::move(id_), std::move(name_), base_price, max_count), healedPoints(healed_points) {}

			HitPoints getHealedPoints(const std::shared_ptr<Player> &) override { return healedPoints; }

		private:
			HitPoints healedPoints;
	};
}
