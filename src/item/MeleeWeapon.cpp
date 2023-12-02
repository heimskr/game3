#include "item/MeleeWeapon.h"

namespace Game3 {
	MeleeWeapon::MeleeWeapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, Durability max_durability):
	Weapon(id_, std::move(name_), base_price, base_damage, max_durability) {
		attributes.emplace("base:attribute/melee_weapon");
	}

	bool MeleeWeapon::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>, Hand hand) {
		return true;
	}
}
