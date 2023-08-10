#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/TeleportSelfPacket.h"

namespace Game3 {
	void TeleportSelfPacket::handle(ServerGame &game, RemoteClient &client) {
		auto player = client.getPlayer();
		if (!player)
			return;

		RealmPtr realm = game.tryRealm(realmID);
		if (!realm)
			return;

		player->teleport(position, realm);
	}
}
