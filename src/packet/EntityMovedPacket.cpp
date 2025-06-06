#include "util/Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityMovedPacket.h"

namespace Game3 {
	EntityMovedPacket::EntityMovedPacket(const Entity &entity):
		EntityMovedPacket(Args{entity.globalID, entity.nextRealm == 0? entity.realmID : entity.nextRealm, entity.getPosition(), entity.direction, entity.offset, entity.velocity, true, false}) {}

	void EntityMovedPacket::encode(Game &, Buffer &buffer) const {
		buffer << arguments.globalID << arguments.realmID << arguments.position << arguments.facing << arguments.offset << arguments.velocity << arguments.adjustOffset << arguments.isTeleport;
	}

	void EntityMovedPacket::decode(Game &, Buffer &buffer) {
		buffer >> arguments.globalID >> arguments.realmID >> arguments.position >> arguments.facing >> arguments.offset >> arguments.velocity >> arguments.adjustOffset >> arguments.isTeleport;
	}

	void EntityMovedPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->tryRealm(arguments.realmID);
		if (!realm) {
			WARN("EntityMovedPacket: Couldn't find realm {} in EntityMovedPacket.", arguments.realmID);
			return;
		}

		EntityPtr entity = game->getAgent<Entity>(arguments.globalID);
		if (!entity) {
			// WARN("EntityMovedPacket: Couldn't find entity {}. Player is {}", arguments.globalID, game->getPlayer()->getGID());
			return;
		}

		const Vector3 offset = entity->offset.copyBase();
		Position position = entity->getPosition();
		const double apparent_x = offset.x + static_cast<double>(position.column);
		const double apparent_y = offset.y + static_cast<double>(position.row);

		MovementContext context{
			.isTeleport = arguments.isTeleport
		};

		if (entity->isInLimbo())
			context.isTeleport = true;

		entity->direction = arguments.facing;
		entity->teleport(arguments.position, realm, context);

		if (!context.isTeleport) {
			if (arguments.adjustOffset) {
				position = entity->getPosition();
				entity->offset.x = apparent_x - position.column;
				entity->offset.y = apparent_y - position.row;
			} else if (arguments.offset)
				entity->offset = *arguments.offset;
		}

		if (arguments.velocity)
			entity->velocity = *arguments.velocity;
	}
}
