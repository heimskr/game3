#include "Log.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "packet/RealmNoticePacket.h"
#include "realm/Realm.h"

namespace Game3 {
	RealmNoticePacket::RealmNoticePacket(Realm &realm):
		RealmNoticePacket(realm.id, realm.type, realm.getTileset().identifier, realm.seed, realm.outdoors) {}

	void RealmNoticePacket::handle(ClientGame &game) {
		if (!game.hasRealm(realmID)) {
			auto realm = Realm::create(game, realmID, type, tileset, seed);
			realm->outdoors = outdoors;
			game.addRealm(realmID, realm);
		}
	}
}
