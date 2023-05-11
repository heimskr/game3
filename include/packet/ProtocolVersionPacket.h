#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ProtocolVersionPacket: Packet {
		static PacketID ID() { return 1; }

		Version version;

		ProtocolVersionPacket(Version version_ = Game::PROTOCOL_VERSION):
			version(version_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << version; }
		void decode(Game &, Buffer &buffer) override { buffer >> version; }
	};
}
