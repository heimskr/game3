#include "entity/Entity.h"
#include "game/ServerGame.h"
#include "net/Buffer.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/PacketError.h"
#include "packet/JumpPacket.h"

namespace Game3 {
	void JumpPacket::handle(const std::shared_ptr<ServerGame> &, GenericClient &client) {
		if (ServerPlayerPtr player = client.getPlayer()) {
			player->jump();
		} else {
			client.sendError("No player.");
		}
	}
}
