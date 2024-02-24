#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct ClickPacket: Packet {
		static PacketID ID() { return 29; }

		Position position;
		double offsetX = 0.;
		double offsetY = 0.;
		Modifiers modifiers;

		ClickPacket() = default;
		ClickPacket(const Position &position_, double offset_x, double offset_y, Modifiers modifiers_):
			position(position_), offsetX(offset_x), offsetY(offset_y), modifiers(modifiers_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << position << offsetX << offsetY << modifiers; }
		void decode(Game &, Buffer &buffer)       override { buffer >> position >> offsetX >> offsetY >> modifiers; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
