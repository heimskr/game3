#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct BuyFromRhosumPacket: Packet {
		static PacketID ID() { return 73; }

		GlobalID globalID = -1;
		Slot index{};

		BuyFromRhosumPacket() = default;
		BuyFromRhosumPacket(GlobalID globalID, uint8_t index):
			globalID(globalID), index(index) {}

		PacketID getID() const final { return ID(); }

		void encode(Game &, Buffer &buffer) const final { buffer << globalID << index; }
		void decode(Game &, Buffer &buffer)       final { buffer >> globalID >> index; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) final;
	};
}
