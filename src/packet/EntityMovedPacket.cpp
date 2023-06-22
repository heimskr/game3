#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityMovedPacket.h"

namespace Game3 {
	EntityMovedPacket::EntityMovedPacket(const Entity &entity):
		EntityMovedPacket(entity.globalID, entity.nextRealm == -1? entity.realmID : entity.nextRealm, entity.getPosition(), entity.direction, entity.offset, entity.zSpeed) {}

	void EntityMovedPacket::handle(ClientGame &game) {
		auto iter = game.realms.find(realmID);
		if (iter == game.realms.end()) {
			WARN("Couldn't find realm " << realmID << " in EntityMovedPacket");
			return;
		}

		auto realm = iter->second;
		auto entity = game.getAgent<Entity>(globalID);

		if (!entity) {
			WARN("Couldn't find entity " << globalID << ". Player is " << game.player->getGID());
			auto lock = game.allAgents.sharedLock();
			WARN("allAgents size: " << game.allAgents.size());
			if (game.allAgents.size() < 2000) {
				for (const auto &[gid, weak]: game.allAgents) {
					if (auto locked = weak.lock()) {
						INFO("    " << gid << ": " << typeid(*locked).name() << " " << locked->getGID());
					} else {
						INFO("    " << gid << ": expired");
					}
				}
			}
			return;
		}

		if (!entity->isPlayer())
			INFO("Moving non-player entity " << globalID << " (" << typeid(*entity).name() << "). Player is " << game.player->getGID());

		entity->direction = facing;
		entity->teleport(position, realm);

		if (offset)
			entity->offset = *offset;

		if (zSpeed)
			entity->zSpeed = *zSpeed;
	}
}
