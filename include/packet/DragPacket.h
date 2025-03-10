#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct DragPacket: Packet {
		enum class Action: uint8_t {Invalid = 0, Start, Update, End};

		static PacketID ID() { return 49; }

		Action action{};
		Position position;
		Modifiers modifiers;
		std::pair<float, float> offsets{};

		DragPacket() = default;
		DragPacket(Action action, const Position &position, Modifiers modifiers, std::pair<float, float> offsets):
			action(action), position(position), modifiers(modifiers), offsets(offsets) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << action << position << modifiers << offsets.first << offsets.second; }
		void decode(Game &, Buffer &buffer)       override { buffer >> action >> position >> modifiers >> offsets.first >> offsets.second; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
