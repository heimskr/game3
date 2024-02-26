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

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &buffer) const final { buffer << slot; }
		void decode(Game &, Buffer &buffer)       final { buffer >> slot; }

		void handle(const std::shared_ptr<ServerGame> &, RemoteClient &) final;
		void handle(const std::shared_ptr<ClientGame> &) final;
	};
}
