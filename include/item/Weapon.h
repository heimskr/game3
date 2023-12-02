#pragma once

#include "item/Tool.h"

namespace Game3 {
	class Weapon: public Tool {
		public:
			HitPoints baseDamage;

			Weapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, Durability max_durability);
	};
}
