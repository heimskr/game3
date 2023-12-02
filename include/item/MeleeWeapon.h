#pragma once

#include "item/Weapon.h"

namespace Game3 {
	class MeleeWeapon: public Weapon {
		public:
			MeleeWeapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, Durability max_durability);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;

		private:
			static Identifier findDirtTilename(const Place &);
	};
}
