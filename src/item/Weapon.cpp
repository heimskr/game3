#include "item/Weapon.h"

namespace Game3 {
	Weapon::Weapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, Durability max_durability):
		Tool(id_, std::move(name_), base_price, 0.f, max_durability, "base:attribute/weapon"_id), baseDamage(base_damage) {}
}
