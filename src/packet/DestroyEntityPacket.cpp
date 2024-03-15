#include "Log.h"
#include "game/ClientGame.h"
#include "packet/DestroyEntityPacket.h"

namespace Game3 {
	DestroyEntityPacket::DestroyEntityPacket(const Entity &entity, bool require_realm):
		globalID(entity.getGID()),
		realmRequirement(require_realm? std::make_optional(entity.getRealm()->id) : std::nullopt) {}

	void DestroyEntityPacket::handle(const ClientGamePtr &game) {
		if (auto entity = game->getAgent<Entity>(globalID)) {
			if (realmRequirement && entity->getRealm()->id != *realmRequirement)
				return;
			if (entity->isPlayer())
				INFO("Destroying player {}", entity->getGID());
			entity->queueDestruction();
		}
		// else WARN("DestroyEntityPacket: couldn't find entity {}.", globalID);
	}
}
