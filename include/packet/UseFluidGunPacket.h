#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct UseFluidGunPacket: Packet {
		static PacketID ID() { return 69; }

		Position position;
		float offsetX = 0.f;
		float offsetY = 0.f;
		Modifiers modifiers;
		uint16_t tickFrequency{};

		UseFluidGunPacket() = default;
		UseFluidGunPacket(const Position &position, float offsetX, float offsetY, Modifiers modifiers, uint16_t tickFrequency):
			position(position), offsetX(offsetX), offsetY(offsetY), modifiers(modifiers), tickFrequency(tickFrequency) {}

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &buffer) const final { buffer << position << offsetX << offsetY << modifiers << tickFrequency; }
		void decode(Game &, BasicBuffer &buffer)       final { buffer >> position >> offsetX >> offsetY >> modifiers >> tickFrequency; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) final;
	};
}
