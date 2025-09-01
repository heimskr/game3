#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ProtocolVersionPacket: Packet {
		constexpr static Version PROTOCOL_VERSION = 14;

		static PacketID ID() { return 1; }

		Version version;

		ProtocolVersionPacket(Version version_ = PROTOCOL_VERSION):
			version(version_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << version; }
		void decode(Game &, BasicBuffer &buffer) override { buffer >> version; }
	};
}
