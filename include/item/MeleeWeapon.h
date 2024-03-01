#pragma once

#include "item/Weapon.h"

namespace Game3 {
	class MeleeWeapon: public Weapon {
		public:
			MeleeWeapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, int variability_, Durability max_durability);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, Hand) override;

		private:
			static Identifier findDirtTilename(const Place &);
	};
}
