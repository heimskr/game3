#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct RegistrationStatusPacket: Packet {
		static PacketID ID() { return 13; }

		Token token;

		RegistrationStatusPacket(Token token_ = 0):
			token(token_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << token; }
		void decode(Game &, Buffer &buffer)       override { buffer >> token; }

		void handle(ClientGame &) const override;
	};
}
