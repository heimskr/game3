#include "Log.h"
#include "game/ClientGame.h"
#include "packet/EntitySetPathPacket.h"

namespace Game3 {
	EntitySetPathPacket::EntitySetPathPacket(Entity &entity):
		EntitySetPathPacket(entity.globalID, entity.realmID, entity.position.copyBase(), entity.copyPath<std::vector>(), entity.getUpdateCounter()) {}

	void EntitySetPathPacket::handle(ClientGame &game) {
		RealmPtr realm = game.tryRealm(realmID);
		if (!realm) {
			ERROR("EntitySetPathPacket: can't find realm " << realmID);
			return;
		}

		EntityPtr entity = game.getAgent<Entity>(globalID);
		if (!entity) {
			ERROR("EntitySetPathPacket: can't find entity " << globalID);
			return;
		}

		if (entity->position.copyBase() != position) {
			entity->teleport(position, MovementContext{
				.isTeleport = true
			});
		}

		entity->path = std::list<Direction>{path.begin(), path.end()};
		entity->setUpdateCounter(newCounter);
	}
}
