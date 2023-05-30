#pragma once

#include "Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ClickPacket: Packet {
		static PacketID ID() { return 29; }

		Position position;

		ClickPacket() = default;
		ClickPacket(const Position &position_):
			position(position_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << position; }
		void decode(Game &, Buffer &buffer)       override { buffer >> position; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
