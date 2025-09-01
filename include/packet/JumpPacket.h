#pragma once

#include <set>
#include <vector>

#include "packet/Packet.h"

namespace Game3 {
	struct JumpPacket: Packet {
		static PacketID ID() { return 38; }

		JumpPacket() = default;

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &) const override {}
		void decode(Game &, BasicBuffer &)       override {}

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
