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

		DragPacket() = default;
		DragPacket(Action action_, const Position &position_, Modifiers modifiers_):
			action(action_), position(position_), modifiers(modifiers_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << action << position << modifiers; }
		void decode(Game &, Buffer &buffer)       override { buffer >> action >> position >> modifiers; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
