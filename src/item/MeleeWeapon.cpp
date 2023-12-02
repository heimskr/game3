#include "entity/Player.h"
#include "item/MeleeWeapon.h"
#include "realm/Realm.h"
#include "types/Position.h"

#include <random>

namespace Game3 {
	MeleeWeapon::MeleeWeapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, Durability max_durability):
	Weapon(id_, std::move(name_), base_price, base_damage, max_durability) {
		attributes.emplace("base:attribute/melee_weapon");
	}

	bool MeleeWeapon::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>, Hand hand) {
		const Position faced_tile = place.position + place.player->direction;
		for (const EntityPtr &entity: place.realm->findEntities(faced_tile)) {
			if (auto living = std::dynamic_pointer_cast<LivingEntity>(entity)) {
				if (living->isInvincible())
					continue;
			}
		}
	}
}
