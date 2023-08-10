#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityMovedPacket.h"

namespace Game3 {
	EntityMovedPacket::EntityMovedPacket(const Entity &entity):
		EntityMovedPacket(entity.globalID, entity.nextRealm == -1? entity.realmID : entity.nextRealm, entity.getPosition(), entity.direction, entity.offset, entity.zSpeed) {}

	void EntityMovedPacket::handle(ClientGame &game) {
		RealmPtr realm = game.tryRealm(realmID);
		if (!realm) {
			WARN("Couldn't find realm " << realmID << " in EntityMovedPacket.");
			return;
		}

		EntityPtr entity = game.getAgent<Entity>(globalID);
		if (!entity) {
			WARN("Couldn't find entity " << globalID << ". Player is " << game.player->getGID());
			return;
		}

		if (!entity->isPlayer()) {
			auto &entity_ref = *entity;
			INFO("Moving non-player entity " << globalID << " (" << typeid(entity_ref).name() << "). Player is " << game.player->getGID());
		} else {
			INFO("Moving " << std::dynamic_pointer_cast<Player>(entity)->displayName << " to " << position << " in realm " << realm->id << ". (GID: " << entity->getGID() << ")");
			if (auto r = entity->weakRealm.lock()) {
				INFO("Said player is in " << r->id << ".");
			}
		}

		entity->direction = facing;
		entity->teleport(position, realm);

		if (entity->isPlayer()) {
			if (auto r = entity->weakRealm.lock()) {
				INFO("Now they're in " << r->id << ".");
			}
		}

		if (offset)
			entity->offset = *offset;

		if (zSpeed)
			entity->zSpeed = *zSpeed;
	}
}
