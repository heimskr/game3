#include "util/Log.h"
#include "entity/Entity.h"
#include "game/ClientGame.h"
#include "packet/EntitySetPathPacket.h"

namespace Game3 {
	EntitySetPathPacket::EntitySetPathPacket(Entity &entity):
		EntitySetPathPacket(entity.globalID, entity.realmID, entity.position.copyBase(), entity.copyPath<std::vector>(), entity.getUpdateCounter()) {}

	void EntitySetPathPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->tryRealm(realmID);
		if (!realm) {
			ERR("EntitySetPathPacket: can't find realm {}", realmID);
			return;
		}

		EntityPtr entity = game->getAgent<Entity>(globalID);
		if (!entity) {
			ERR("EntitySetPathPacket: can't find entity {}", globalID);
			return;
		}

		if (entity->position.copyBase() != position) {
			const Position current_position = entity->getPosition();

			Vector3 offset = entity->offset.copyBase();
			offset.x += current_position.column - position.column;
			offset.y += current_position.row - position.row;

			entity->teleport(position, MovementContext{
				.forcedOffset = offset,
				.isTeleport = true
			});
		}

		entity->path = std::deque<Direction>{path.begin(), path.end()};
		entity->setUpdateCounter(newCounter);
	}
}
