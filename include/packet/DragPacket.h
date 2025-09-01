#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct DragPacket: Packet {
		static PacketID ID() { return 49; }

		DragAction action{};
		Position position;
		Modifiers modifiers;
		std::pair<float, float> offsets{};

		DragPacket() = default;
		DragPacket(DragAction action, const Position &position, Modifiers modifiers, std::pair<float, float> offsets):
			action(action), position(position), modifiers(modifiers), offsets(offsets) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << action << position << modifiers << offsets.first << offsets.second; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> action >> position >> modifiers >> offsets.first >> offsets.second; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
