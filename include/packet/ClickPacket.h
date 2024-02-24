#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct ClickPacket: Packet {
		static PacketID ID() { return 29; }

		Position position;
		float offsetX = 0.f;
		float offsetY = 0.f;
		Modifiers modifiers;

		ClickPacket() = default;
		ClickPacket(const Position &position_, float offset_x, float offset_y, Modifiers modifiers_):
			position(position_), offsetX(offset_x), offsetY(offset_y), modifiers(modifiers_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << position << offsetX << offsetY << modifiers; }
		void decode(Game &, Buffer &buffer)       override { buffer >> position >> offsetX >> offsetY >> modifiers; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
