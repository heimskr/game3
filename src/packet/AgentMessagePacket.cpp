#include "Log.h"
#include "entity/ClientPlayer.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/AgentMessagePacket.h"

namespace Game3 {
	void AgentMessagePacket::handle(const std::shared_ptr<ServerGame> &game, RemoteClient &client) {
		if (!Agent::validateGID(globalID)) {
			client.send(ErrorPacket("Can't send message to agent: invalid GID"));
			return;
		}

		AgentPtr destination = game->getAgent(globalID);
		if (!destination) {
			client.send(ErrorPacket("Can't send message to agent: agent not found"));
			return;
		}

		std::any data{std::move(messageData)};
		destination->handleMessage(client.getPlayer(), messageName, data);
	}

	void AgentMessagePacket::handle(const ClientGamePtr &game) {
		if (!Agent::validateGID(globalID)) {
			ERROR_("Can't send message to player: invalid GID");
			return;
		}

		AgentPtr source = game->getAgent(globalID);
		if (!source) {
			ERROR_("Can't send message to player: agent not found");
			return;
		}

		std::any data{std::move(messageData)};
		game->getPlayer()->handleMessage(source, messageName, data);
	}
}
