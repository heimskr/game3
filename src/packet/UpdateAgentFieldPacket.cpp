#include "game/Agent.h"
#include "game/ClientGame.h"
#include "packet/UpdateAgentFieldPacket.h"

namespace Game3 {
	void UpdateAgentFieldPacket::encode(Game &, Buffer &buffer) const {
		buffer << globalID << fieldNameHash << fieldValue;
	}

	void UpdateAgentFieldPacket::decode(Game &game, Buffer &buffer) {
		fieldValue.context = game.shared_from_this();
		buffer >> globalID >> fieldNameHash >> fieldValue;
	}

	void UpdateAgentFieldPacket::handle(const std::shared_ptr<ClientGame> &game) {
		AgentPtr agent = game->getAgent(globalID);
		if (!agent) {
			WARN("Couldn't find agent {}", globalID);
			return;
		}

		agent->setField(fieldNameHash, std::move(fieldValue));
	}
}
