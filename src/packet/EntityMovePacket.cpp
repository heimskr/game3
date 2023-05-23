#include "Log.h"
#include "game/ClientGame.h"
#include "packet/EntityMovePacket.h"

namespace Game3 {
	EntityMovePacket::EntityMovePacket(const EntityPtr &entity):
		EntityMovePacket(entity->globalID, entity->nextRealm == -1? entity->realmID : entity->nextRealm, entity->getPosition(), entity->direction, entity->offset.x(), entity->offset.y()) {}

	void EntityMovePacket::handle(ClientGame &game) {
		auto iter = game.realms.find(realmID);
		if (iter == game.realms.end()) {
			WARN("Couldn't find realm " << realmID);
			return;
		}

		auto realm = iter->second;
		auto entity = realm->getEntity(globalID);

		if (!entity) {
			auto lock = game.lockAllEntitiesShared();
			if (auto iter = game.allEntities.find(globalID); iter != game.allEntities.end())
				entity = iter->second;
			else
				return;
		}

		if (!dynamic_cast<Player *>(entity.get())) {
			INFO("Moving non-player entity");
		}

		entity->direction = facing;
		entity->teleport(position, realm);

		if (xOffset)
			entity->offset.x() = *xOffset;

		if (yOffset)
			entity->offset.y() = *yOffset;
	}
}
