#include "entity/Player.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/BedTile.h"
#include "types/Position.h"

namespace Game3 {
	void BedTile::jumpedFrom(const EntityPtr &, const Place &place, Layer) {
		if (PlayerPtr player = place.player; player && player->getSide() == Side::Server) {
			player->spawnRealmID = place.realm->getID();
			player->spawnPosition = place.position;
			place.realm->playSound(place.position, "base:sound/spawn_set", threadContext.getPitch(1.05f));
		}
	}
}
