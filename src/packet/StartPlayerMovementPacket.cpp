#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/StartPlayerMovementPacket.h"

namespace Game3 {
	void StartPlayerMovementPacket::handle(ServerGame &, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			player->path.clear();
			player->startMoving(direction);
			return;
		}

		client.send(ErrorPacket("Can't move: no player"));
	}
}
