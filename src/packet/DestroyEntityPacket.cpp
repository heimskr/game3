#include "Log.h"
#include "game/ClientGame.h"
#include "packet/DestroyEntityPacket.h"

namespace Game3 {
	DestroyEntityPacket::DestroyEntityPacket(const Entity &entity):
		DestroyEntityPacket(entity.getGID(), entity.realmID) {}

	void DestroyEntityPacket::handle(ClientGame &game) {
		auto realm_iter = game.realms.find(realmID);

		if (realm_iter == game.realms.end()) {
			WARN("DestroyEntityPacket: couldn't find realm " << realmID);
			return;
		}

		auto realm = realm_iter->second;
		auto entity = realm->getEntity(globalID);

		if (!entity) {
			WARN("DestroyEntityPacket: couldn't find entity " << globalID << " in realm " << realmID);
			return;
		}

		entity->destroy();
	}
}
