#include "util/Log.h"
#include "entity/LivingEntity.h"
#include "game/ClientGame.h"
#include "packet/LivingEntityHealthChangedPacket.h"

namespace Game3 {
	LivingEntityHealthChangedPacket::LivingEntityHealthChangedPacket(const LivingEntity &living_entity):
		globalID(living_entity.getGID()),
		newHealth(living_entity.health) {}

	void LivingEntityHealthChangedPacket::handle(const ClientGamePtr &game) {
		if (auto entity = game->getAgent<LivingEntity>(globalID))
			entity->setHealth(newHealth);
	}
}
