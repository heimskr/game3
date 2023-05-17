#include "Log.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/StopPlayerMovementPacket.h"

namespace Game3 {
	void StopPlayerMovementPacket::handle(ServerGame &, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			player->path.clear();
			if (direction)
				player->stopMoving(*direction);
			else
				player->stopMoving();
			return;
		}

		client.send(ErrorPacket("Can't move: no player"));
	}
}
