#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ErrorPacket: Packet {
		static PacketID ID() { return 16; }

		std::string error;

		ErrorPacket() = default;
		ErrorPacket(std::string_view error):
			error(error) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << error; }
		void decode(Game &, Buffer &buffer)       override { buffer >> error; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
