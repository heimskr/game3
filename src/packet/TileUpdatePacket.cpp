#include "Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/TileUpdatePacket.h"

namespace Game3 {
	void TileUpdatePacket::handle(ClientGame &game) {
		auto realm = game.realms.at(realmID);
		realm->setTile(layer, position, tileID, false);
		realm->reupload(layer);
	}
}
