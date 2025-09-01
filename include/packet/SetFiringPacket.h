#pragma once

#include "types/Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct SetFiringPacket: Packet {
		static PacketID ID() { return 68; }

		bool firing{};

		SetFiringPacket() = default;
		SetFiringPacket(bool firing):
			firing(firing) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << firing; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> firing; }

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
