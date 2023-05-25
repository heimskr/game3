#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ActiveSlotSetPacket: Packet {
		static PacketID ID() { return 26; }

		Slot slot = -1;

		ActiveSlotSetPacket() = default;
		ActiveSlotSetPacket(Slot slot_):
			slot(slot_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << slot; }
		void decode(Game &, Buffer &buffer)       override { buffer >> slot; }

		void handle(ClientGame &) override;
	};
}
