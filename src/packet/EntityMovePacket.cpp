#include "Log.h"
#include "game/ClientGame.h"
#include "packet/EntityMovePacket.h"

namespace Game3 {
	EntityMovePacket::EntityMovePacket(const EntityPtr &entity):
		EntityMovePacket(entity->globalID, entity->realmID, entity->getPosition(), entity->offset.x(), entity->offset.y()) {}

	void EntityMovePacket::handle(ClientGame &game) {
		auto iter = game.realms.find(realmID);
		if (iter == game.realms.end()) {
			// WARN("EntityMovePacket: couldn't find realm " << realmID);
			return;
		}

		auto realm = iter->second;
		auto entity = realm->getEntity(globalID);
		if (!entity) {
			// WARN("EntityMovePacket: Couldn't find entity " << globalID << " in realm " << realmID);
			return;
		}

		entity->teleport(position, realm);

		if (xOffset)
			entity->offset.x() = *xOffset;

		if (yOffset)
			entity->offset.y() = *yOffset;
	}
}
