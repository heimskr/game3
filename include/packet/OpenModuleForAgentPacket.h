#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct OpenModuleForAgentPacket: Packet {
		static PacketID ID() { return 40; }

		Identifier moduleID;
		GlobalID agentGID = -1;
		bool removeOnMove = true;

		OpenModuleForAgentPacket() = default;
		OpenModuleForAgentPacket(Identifier module_id, GlobalID agent_gid, bool remove_on_move = true):
			moduleID(std::move(module_id)), agentGID(agent_gid), removeOnMove(remove_on_move) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << moduleID << agentGID << removeOnMove; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> moduleID >> agentGID >> removeOnMove; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
