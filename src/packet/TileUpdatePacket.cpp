#include "Log.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/TileUpdatePacket.h"

namespace Game3 {
	void TileUpdatePacket::handle(ClientGame &game) {
		game.realms.at(realmID)->setTile(layer, position, tileID, false);
	}
}
