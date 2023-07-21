#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct OpenAgentInventoryPacket: Packet {
		static PacketID ID() { return 40; }

		GlobalID globalID = -1;

		OpenAgentInventoryPacket() = default;
		OpenAgentInventoryPacket(GlobalID global_id):
			globalID(global_id) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID; }

		void handle(ClientGame &) override;
	};
}
