#include "game/ServerGame.h"
#include "net/Server.h"
#include "net/RemoteClient.h"
#include "packet/CommandPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void CommandPacket::handle(const std::shared_ptr<ServerGame> &game, RemoteClient &client) {
		game.runCommand(client, command, commandID);
	}
}
