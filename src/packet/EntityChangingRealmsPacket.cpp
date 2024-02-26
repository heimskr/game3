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
			WARN_("Couldn't find entity " << globalID << "; can't put in limbo for realm " << newRealmID << '.');
			return;
		}

		if (game->hasRealm(newRealmID)) {
			WARN_("Client was sent an EntityChangingRealmsPacket for entity " << globalID << " in realm " << newRealmID << " despite knowing about the realm already");
		}

		game->putInLimbo(entity, newRealmID, newPosition);
	}
}
