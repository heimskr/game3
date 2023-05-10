#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ProtocolVersionPacket: Packet {
		Version version;

		ProtocolVersionPacket(Version version_ = Game::PROTOCOL_VERSION):
			version(version_) {}

		PacketID getID() const override { return 1; }

		void encode(Game &, Buffer &buffer) override { buffer << version; }
		void decode(Game &, Buffer &buffer) override { buffer >> version; }
	};
}
