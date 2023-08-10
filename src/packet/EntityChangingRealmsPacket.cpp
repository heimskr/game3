#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityChangingRealmsPacket.h"

namespace Game3 {
	void EntityChangingRealmsPacket::handle(ClientGame &game) {
		EntityPtr entity = game.getAgent<Entity>(globalID);
		if (!entity) {
			WARN("Couldn't find entity " << globalID);
			return;
		}

		if (!game.hasRealm(newRealmID)) {
			INFO("Putting entity " << entity->getGID() << " in limbo.");
			game.putInLimbo(entity, newRealmID, newPosition);
		}
	}
}
