#include "algorithm/DamageCalculation.h"
#include "entity/Player.h"
#include "item/MeleeWeapon.h"
#include "realm/Realm.h"
#include "types/Position.h"

#include <random>

namespace Game3 {
	MeleeWeapon::MeleeWeapon(ItemID id_, std::string name_, MoneyCount base_price, HitPoints base_damage, int variability_, Durability max_durability):
	Weapon(id_, std::move(name_), base_price, base_damage, variability_, max_durability) {
		attributes.emplace("base:attribute/melee_weapon");
	}

	bool MeleeWeapon::use(Slot, ItemStack &, const Place &place, Modifiers, std::pair<float, float>, Hand hand) {
		if (hand == Hand::None)
			return false;

		PlayerPtr player = place.player;

		if (!player->canAttack())
			return false;

		const Position faced_tile = place.position + player->direction;

		for (const EntityPtr &entity: place.realm->findEntities(faced_tile)) {
			if (auto living = std::dynamic_pointer_cast<LivingEntity>(entity)) {
				if (living->isInvincible())
					continue;

				player->timeSinceAttack = 0;
				const HitPoints damage = calculateDamage(baseDamage, variability, player->getLuck());
				living->takeDamage(damage);
				living->onAttack(player);
				return true;
			}
		}

		return false;
	}
}
