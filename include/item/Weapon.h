#pragma once

#include "item/Tool.h"

namespace Game3 {
	class Weapon: public Tool {
		public:
			HitPoints baseDamage;
			int variability;

			Weapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, int variability_, Durability max_durability);
	};
}
