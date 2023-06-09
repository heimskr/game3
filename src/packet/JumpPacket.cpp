#include "entity/Entity.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/PacketError.h"
#include "packet/JumpPacket.h"

namespace Game3 {
	void JumpPacket::handle(ServerGame &game, RemoteClient &client) {
		if (auto player = client.getPlayer()) {
			player->jump();
		} else {
			client.send(ErrorPacket("No player."));
		}
	}
}
