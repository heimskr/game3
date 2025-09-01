#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct SendChatMessagePacket: Packet {
		static PacketID ID() { return 18; }

		std::string message;

		SendChatMessagePacket() = default;
		SendChatMessagePacket(std::string_view message_):
			message(message_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << message; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> message; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
