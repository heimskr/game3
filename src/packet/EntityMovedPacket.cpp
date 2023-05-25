#include "Log.h"
#include "game/ClientGame.h"
#include "packet/EntityMovedPacket.h"

namespace Game3 {
	EntityMovedPacket::EntityMovedPacket(const Entity &entity):
		EntityMovedPacket(entity.globalID, entity.nextRealm == -1? entity.realmID : entity.nextRealm, entity.realmID, entity.getPosition(), entity.direction, entity.offset.x(), entity.offset.y()) {}

	void EntityMovedPacket::handle(ClientGame &game) {
		auto iter = game.realms.find(realmID);
		if (iter == game.realms.end()) {
			WARN("Couldn't find realm " << realmID);
			return;
		}

		auto realm = iter->second;
		auto entity = realm->getEntity(globalID);

		if (!entity) {
			if (previousRealm != -1)
				if (auto previous_realm = game.realms.find(previousRealm); previous_realm != game.realms.end())
					entity = previous_realm->second->getEntity(globalID);

			if (!entity) {
				WARN("Couldn't find entity " << globalID << " anywhere, in either realm " << realmID << " or realm " << previousRealm);
				return;
			}
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
