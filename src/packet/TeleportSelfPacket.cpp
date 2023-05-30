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

		auto iter = game.realms.find(realmID);
		if (iter == game.realms.end())
			return;

		auto realm = iter->second;
		player->teleport(position, realm);
	}
}
