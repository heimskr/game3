#include "Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/HeldItemSetPacket.h"

namespace Game3 {
	void HeldItemSetPacket::handle(ClientGame &game) {
		RealmPtr realm = game.tryRealm(realmID);
		if (!realm) {
			ERROR("Couldn't find realm " << realmID << " in HeldItemSetPacket handler");
			return;
		}

		EntityPtr entity = realm->getEntity(entityID);
		if (!entity) {
			ERROR("Couldn't find entity " << entityID << " in realm " << realmID << " in HeldItemSetPacket handler");
			return;
		}

		if (leftHand)
			entity->setHeldLeft(slot);
		else
			entity->setHeldRight(slot);

		entity->setUpdateCounter(newCounter);
	}
}
