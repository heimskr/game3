#include "util/Log.h"
#include "entity/Entity.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/HeldItemSetPacket.h"

namespace Game3 {
	void HeldItemSetPacket::handle(const ClientGamePtr &game) {
		RealmPtr realm = game->tryRealm(realmID);
		if (!realm) {
			ERR("Couldn't find realm {} in HeldItemSetPacket handler", realmID);
			return;
		}

		EntityPtr entity = realm->getEntity(entityID);
		if (!entity) {
			ERR("Couldn't find entity {} in realm {} in HeldItemSetPacket handler", entityID, realmID);
			return;
		}

		if (leftHand)
			entity->setHeldLeft(slot);
		else
			entity->setHeldRight(slot);

		entity->setUpdateCounter(newCounter);
	}
}
