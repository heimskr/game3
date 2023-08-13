#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

#include <optional>

namespace Game3 {
	struct MovePlayerPacket: Packet {
		static PacketID ID() { return 15; }

		Position position;
		Direction movementDirection;
		std::optional<Direction> facingDirection;

		MovePlayerPacket() = default;
		MovePlayerPacket(const Position &position_, Direction movement_direction, std::optional<Direction> facing_direction = {}):
			position(position_), movementDirection(movement_direction), facingDirection(facing_direction) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << position << movementDirection << facingDirection; }
		void decode(Game &, Buffer &buffer)       override { buffer >> position >> movementDirection >> facingDirection; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
