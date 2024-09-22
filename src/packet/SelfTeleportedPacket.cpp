#include "Log.h"
#include "entity/ClientPlayer.h"
#include "error/PlayerMissingError.h"
#include "game/ClientGame.h"
#include "packet/SelfTeleportedPacket.h"

namespace Game3 {
	void SelfTeleportedPacket::handle(const ClientGamePtr &game) {
		auto player = game->getPlayer();

		if (!player)
			throw PlayerMissingError("Can't teleport self: player missing");

		RealmPtr realm = game->getRealm(realmID);
		player->teleport(position, realm);
		player->focus(*game->getWindow(), true); // TODO: probably replace `true` with `false`
	}
}
