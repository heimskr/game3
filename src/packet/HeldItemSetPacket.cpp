#include "Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/HeldItemSetPacket.h"

namespace Game3 {
	void HeldItemSetPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->tryRealm(realmID);
		if (!realm) {
			ERROR_("Couldn't find realm " << realmID << " in HeldItemSetPacket handler");
			return;
		}

		EntityPtr entity = realm->getEntity(entityID);
		if (!entity) {
			ERROR_("Couldn't find entity " << entityID << " in realm " << realmID << " in HeldItemSetPacket handler");
			return;
		}

		if (leftHand)
			entity->setHeldLeft(slot);
		else
			entity->setHeldRight(slot);

		entity->setUpdateCounter(newCounter);
	}
}
