#include "util/Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/TileUpdatePacket.h"

namespace Game3 {
	void TileUpdatePacket::handle(const ClientGamePtr &game) {
		auto realm = game->getRealm(realmID);
		realm->setTile(layer, position, tileID, true);
		realm->queueReupload();
	}
}
