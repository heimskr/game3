#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "types/Types.h"

#include <string>

namespace Game3 {
	struct RegisterPlayerPacket: Packet {
		static PacketID ID() { return 12; }

		std::string username;
		std::string displayName;

		RegisterPlayerPacket() = default;
		RegisterPlayerPacket(std::string_view username_, std::string_view display_name):
			username(username_), displayName(display_name) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << username << displayName; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> username >> displayName; }

		void handle(const ServerGamePtr &, GenericClient &) override;
	};
}
