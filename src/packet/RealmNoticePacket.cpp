#include "Log.h"
#include "game/ClientGame.h"
#include "packet/RealmNoticePacket.h"
#include "realm/Realm.h"

namespace Game3 {
	void RealmNoticePacket::handle(ClientGame &game) {
		if (!game.realms.contains(realmID)) {
			INFO("Adding realm " << realmID << " of type " << type << " with tileset " << tileset);
			game.realms.emplace(realmID, Realm::create(game, realmID, type, tileset, seed));
		}
	}
}
