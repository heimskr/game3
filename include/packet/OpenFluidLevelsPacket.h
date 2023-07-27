#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct OpenFluidLevelsPacket: Packet {
		static PacketID ID() { return 43; }

		GlobalID globalID = -1;

		OpenFluidLevelsPacket() = default;
		OpenFluidLevelsPacket(GlobalID global_id):
			globalID(global_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID; }

		void handle(ClientGame &) override;
	};
}
