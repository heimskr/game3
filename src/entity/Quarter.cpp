#include "entity/Quarter.h"

namespace Game3 {
	void Quarter::onSpawn() {
		Animal::onSpawn();
		setHealth(1);
		spawnBlood(20);
	}

	void Quarter::tick(const TickArgs &args) {
		// Not much this entity can do.
		LivingEntity::tick(args);
	}
}
