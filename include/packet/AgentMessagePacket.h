#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

namespace Game3 {
	struct AgentMessagePacket: Packet {
		static PacketID ID() { return 43; }

		GlobalID globalID = -1;
		std::string messageName;
		Buffer messageData;

		AgentMessagePacket() = default;
		AgentMessagePacket(GlobalID global_id, std::string message_name, Buffer message_data):
			globalID(global_id), messageName(std::move(message_name)), messageData(std::move(message_data)) {}

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &buffer) const final { buffer << globalID << messageName << messageData; }
		void decode(Game &, Buffer &buffer)       final { buffer >> globalID >> messageName >> messageData; }

		void handle(const std::shared_ptr<ServerGame> &, RemoteClient &) final;
		void handle(const std::shared_ptr<ClientGame> &) final;
	};
}
