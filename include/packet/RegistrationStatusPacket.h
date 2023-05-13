#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct RegistrationStatusPacket: Packet {
		static PacketID ID() { return 13; }

		std::string username;
		std::string displayName;
		Token token = 0;

		RegistrationStatusPacket() = default;
		RegistrationStatusPacket(std::string_view username_, std::string_view display_name, Token token_ = 0):
			username(username_), displayName(display_name), token(token_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << username << displayName << token; }
		void decode(Game &, Buffer &buffer)       override { buffer >> username >> displayName >> token; }

		void handle(ClientGame &) const override;
	};
}
