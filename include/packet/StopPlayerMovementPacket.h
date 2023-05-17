#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

namespace Game3 {
	struct StopPlayerMovementPacket: Packet {
		static PacketID ID() { return 18; }

		std::optional<Direction> direction;

		StopPlayerMovementPacket() = default;
		StopPlayerMovementPacket(Direction direction_):
			direction(direction_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << direction; }
		void decode(Game &, Buffer &buffer)       override { buffer >> direction; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}