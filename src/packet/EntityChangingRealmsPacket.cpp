#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityChangingRealmsPacket.h"

namespace Game3 {
	void EntityChangingRealmsPacket::handle(const ClientGamePtr &game) {
		if (globalID == game->getPlayer()->getGID())
			return;

		EntityPtr entity = game->getAgent<Entity>(globalID);
		if (!entity) {
			WARN("Couldn't find entity {}; can't put in limbo for realm {}.", globalID, newRealmID);
			return;
		}

		if (game->hasRealm(newRealmID)) {
			WARN("Client was sent an EntityChangingRealmsPacket for entity {} in realm {} despite knowing about the realm already", globalID, newRealmID);
		}

		game->putInLimbo(entity, newRealmID, newPosition);
	}
}
