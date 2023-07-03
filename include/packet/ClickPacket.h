#pragma once

#include "Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ClickPacket: Packet {
		static PacketID ID() { return 29; }

		Position position;
		float offsetX = 0.f;
		float offsetY = 0.f;

		ClickPacket() = default;
		ClickPacket(const Position &position_, float offset_x, float offset_y):
			position(position_), offsetX(offset_x), offsetY(offset_y) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << position << offsetX << offsetY; }
		void decode(Game &, Buffer &buffer)       override { buffer >> position >> offsetX >> offsetY; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
