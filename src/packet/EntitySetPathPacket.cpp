#include "Log.h"
#include "game/ClientGame.h"
#include "packet/EntitySetPathPacket.h"

namespace Game3 {
	EntitySetPathPacket::EntitySetPathPacket(const Entity &entity):
		EntitySetPathPacket(entity.globalID, entity.realmID, entity.getPosition(), {entity.path.begin(), entity.path.end()}) {}

	void EntitySetPathPacket::handle(ClientGame &game) {
		auto iter = game.realms.find(realmID);
		if (iter == game.realms.end())
			return;

		auto realm = iter->second;
		auto entity = realm->getEntity(globalID);
		if (!entity)
			return;

		entity->path = {path.begin(), path.end()};
	}
}
