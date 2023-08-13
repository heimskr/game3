#include "Log.h"
#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "packet/EntityChangingRealmsPacket.h"

namespace Game3 {
	void EntityChangingRealmsPacket::handle(ClientGame &game) {
		if (globalID == game.player->getGID())
			return;

		EntityPtr entity = game.getAgent<Entity>(globalID);
		if (!entity) {
			WARN("Couldn't find entity " << globalID << "; can't put in limbo for realm " << newRealmID << '.');
			return;
		}

		game.putInLimbo(entity, newRealmID, newPosition);
	}
}
