#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void RegisterPlayerPacket::handle(ServerGame &game, RemoteClient &client) const {
		if (game.server->hasUsername(username)) {

		}
	}
}
