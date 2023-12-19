#include "Log.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "packet/RealmNoticePacket.h"
#include "realm/Realm.h"
#include "realm/RealmFactory.h"

namespace Game3 {
	RealmNoticePacket::RealmNoticePacket(Realm &realm):
		RealmNoticePacket(realm.id, realm.type, realm.getTileset().identifier, realm.seed, realm.outdoors) {}

	void RealmNoticePacket::handle(ClientGame &game) {
		if (!game.hasRealm(realmID)) {
			auto factory = game.registry<RealmFactoryRegistry>().at(type);
			assert(factory);
			auto realm = (*factory)(game);
			realm->id = realmID;
			realm->type = type;
			realm->tileProvider.tilesetID = tileset;
			realm->seed = seed;
			realm->outdoors = outdoors;
			game.addRealm(realmID, realm);
		}
	}
}
