#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityChangingRealmsPacket.h"

namespace Game3 {
	void EntityChangingRealmsPacket::handle(ClientGame &game) {
		if (globalID == game.player->getGID())
			return;

		INFO("Entity changing realms: " << globalID);

		EntityPtr entity = game.getAgent<Entity>(globalID);
		if (!entity) {
			WARN("Couldn't find entity " << globalID << "; can't put in limbo for realm " << newRealmID << '.');
			return;
		}

		if (game.hasRealm(newRealmID)) {
			WARN("Client was sent an EntityChangingRealmsPacket for entity " << globalID << " in realm " << newRealmID << " despite knowing about the realm already");
		}

		INFO("Putting " << entity->getGID() << " in limbo for realm " << newRealmID);
		game.putInLimbo(entity, newRealmID, newPosition);
	}
}
