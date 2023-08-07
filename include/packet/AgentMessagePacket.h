#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

namespace Game3 {
	struct AgentMessagePacket: Packet {
		static PacketID ID() { return 43; }

		GlobalID destinationGID = -1;
		std::string messageName;
		Buffer messageData;

		AgentMessagePacket() = default;
		AgentMessagePacket(GlobalID destination_gid, std::string message_name, Buffer message_data):
			destinationGID(destination_gid), messageName(std::move(message_name)), messageData(std::move(message_data)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << destinationGID << messageName << messageData; }
		void decode(Game &, Buffer &buffer)       override { buffer >> destinationGID >> messageName >> messageData; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
