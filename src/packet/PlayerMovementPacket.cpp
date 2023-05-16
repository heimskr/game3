#include "Log.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/PlayerMovementPacket.h"

namespace Game3 {
	void PlayerMovementPacket::handle(ServerGame &, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			player->path.clear();
			if (direction) {
				INFO("Starting to move in direction " << *direction);
				player->startMoving(*direction);
			} else {
				INFO("Stopping movement");
				player->stopMoving();
			}
			return;
		}

		client.send(ErrorPacket("Can't move: no player"));
	}
}
