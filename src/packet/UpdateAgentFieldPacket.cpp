#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ServerGame.h"
#include "net/GenericClient.h"
#include "packet/UpdateAgentFieldPacket.h"

namespace Game3 {
	void UpdateAgentFieldPacket::encode(Game &, Buffer &buffer) const {
		buffer << globalID << fieldNameHash << fieldValue;
	}

	void UpdateAgentFieldPacket::decode(Game &game, BasicBuffer &buffer) {
		fieldValue.context = game.shared_from_this();
		buffer >> globalID >> fieldNameHash >> fieldValue;
	}

	void UpdateAgentFieldPacket::handle(const std::shared_ptr<ServerGame> &game, GenericClient &client) {
		AgentPtr agent = game->getAgent(globalID);
		if (!agent) {
			WARN("Couldn't find agent {}", globalID);
			return;
		}

		agent->setField(fieldNameHash, fieldValue, client.getPlayer());
	}

	void UpdateAgentFieldPacket::handle(const std::shared_ptr<ClientGame> &game) {
		AgentPtr agent = game->getAgent(globalID);
		if (!agent) {
			WARN("Couldn't find agent {}", globalID);
			return;
		}

		agent->setField(fieldNameHash, fieldValue, nullptr);
	}
}
