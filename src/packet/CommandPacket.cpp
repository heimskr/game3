#include "game/ServerGame.h"
#include "net/LocalServer.h"
#include "net/RemoteClient.h"
#include "packet/CommandPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void CommandPacket::handle(ServerGame &game, RemoteClient &client) const {
		game.runCommand(client, command, commandID);
	}
}
