#include "entity/LivingEntity.h"
#include "game/ClientGame.h"
#include "packet/StatusEffectsPacket.h"

namespace Game3 {
	StatusEffectsPacket::StatusEffectsPacket(LivingEntity &entity):
		StatusEffectsPacket(entity.getGID(), entity.copyStatusEffects()) {}

	StatusEffectsPacket::StatusEffectsPacket(GlobalID globalID, StatusEffectMap map):
		globalID(globalID),
		map(std::move(map)) {}

	void StatusEffectsPacket::handle(const std::shared_ptr<ClientGame> &game) {
		LivingEntityPtr entity = game->getAgent<LivingEntity>(globalID);
		if (!entity) {
			WARN("Couldn't find living entity {}", globalID);
			return;
		}

		entity->setStatusEffects(std::move(map));
	}
}
