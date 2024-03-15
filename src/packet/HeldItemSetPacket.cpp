#include "Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/HeldItemSetPacket.h"

namespace Game3 {
	void HeldItemSetPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->tryRealm(realmID);
		if (!realm) {
			ERROR("Couldn't find realm {} in HeldItemSetPacket handler", realmID);
			return;
		}

		EntityPtr entity = realm->getEntity(entityID);
		if (!entity) {
			ERROR("Couldn't find entity {} in realm {} in HeldItemSetPacket handler", entityID, realmID);
			return;
		}

		if (leftHand)
			entity->setHeldLeft(slot);
		else
			entity->setHeldRight(slot);

		entity->setUpdateCounter(newCounter);
	}
}
