#include "Log.h"
#include "entity/ServerPlayer.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/AgentMessagePacket.h"

namespace Game3 {
	void AgentMessagePacket::handle(ServerGame &game, RemoteClient &client) {
		if (!Agent::validateGID(destinationGID)) {
			client.send(ErrorPacket("Can't send message to agent: invalid GID"));
			return;
		}

		AgentPtr destination = game.getAgent(destinationGID);
		if (!destination) {
			client.send(ErrorPacket("Can't send message to agent: agent not found"));
			return;
		}

		destination->handleMessage(*client.getPlayer(), messageName, messageData);
	}
}
