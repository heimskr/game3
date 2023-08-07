#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/AgentMessagePacket.h"

namespace Game3 {
	void AgentMessagePacket::handle(ServerGame &game, RemoteClient &client) {
		if (!Agent::validateGID(globalID)) {
			client.send(ErrorPacket("Can't send message to agent: invalid GID"));
			return;
		}

		AgentPtr destination = game.getAgent(globalID);
		if (!destination) {
			client.send(ErrorPacket("Can't send message to agent: agent not found"));
			return;
		}

		destination->handleMessage(*client.getPlayer(), messageName, messageData);
	}

	void AgentMessagePacket::handle(ClientGame &game) {
		if (!Agent::validateGID(globalID)) {
			ERROR("Can't send message to player: invalid GID");
			return;
		}

		AgentPtr source = game.getAgent(globalID);
		if (!source) {
			ERROR("Can't send message to player: agent not found");
			return;
		}

		game.player->handleMessage(*source, messageName, messageData);
	}
}
