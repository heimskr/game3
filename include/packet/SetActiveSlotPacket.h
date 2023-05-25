#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct SetActiveSlotPacket: Packet {
		static PacketID ID() { return 25; }

		Slot slot = -1;

		SetActiveSlotPacket() = default;
		SetActiveSlotPacket(Slot slot_):
			slot(slot_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << slot; }
		void decode(Game &, Buffer &buffer)       override { buffer >> slot; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
