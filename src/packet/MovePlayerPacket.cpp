#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/MovePlayerPacket.h"

namespace Game3 {
	void MovePlayerPacket::handle(ServerGame &, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			player->path.clear();
			player->move(movementDirection, {.excludePlayerSelf = true, .clearOffset = false, .facingDirection = facingDirection, .forcedPosition = position});
			return;
		}

		client.send(ErrorPacket("Can't move: no player"));
	}
}
