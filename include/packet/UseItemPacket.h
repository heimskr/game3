#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct UseItemPacket: Packet {
		static PacketID ID() { return 55; }

		Slot slot;
		Modifiers modifiers;

		UseItemPacket() = default;
		UseItemPacket(Slot slot_, Modifiers modifiers_):
			slot(slot_), modifiers(modifiers_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << slot << modifiers; }
		void decode(Game &, Buffer &buffer)       override { buffer >> slot >> modifiers; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
