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
		ClickPacket(const Position &position, float offsetX, float offsetY, Modifiers modifiers):
			position(position), offsetX(offsetX), offsetY(offsetY), modifiers(modifiers) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << position << offsetX << offsetY << modifiers; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> position >> offsetX >> offsetY >> modifiers; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
