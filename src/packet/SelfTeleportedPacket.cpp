#include "Log.h"
#include "entity/ClientPlayer.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/SelfTeleportedPacket.h"

namespace Game3 {
	void SelfTeleportedPacket::handle(ClientGame &game) {
		if (!game.player)
			throw PlayerMissingError("Can't teleport self: player missing");

		game.player->teleport(position, game.getRealm(realmID));
		game.player->focus(game.canvas, true);
	}
}
