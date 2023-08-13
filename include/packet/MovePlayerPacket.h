#pragma once

#include "game/Game.h"
#include "packet/Packet.h"

#include <optional>

namespace Game3 {
	struct MovePlayerPacket: Packet {
		static PacketID ID() { return 15; }

		Position position;
		Direction movementDirection = Direction::Invalid;
		std::optional<Direction> facingDirection;
		std::optional<Offset> offset;

		MovePlayerPacket() = default;
		MovePlayerPacket(const Position &position_, Direction movement_direction, std::optional<Direction> facing_direction = {}, std::optional<Offset> offset_ = std::nullopt);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << position << movementDirection << facingDirection << offset; }
		void decode(Game &, Buffer &buffer)       override { buffer >> position >> movementDirection >> facingDirection >> offset; }

		void handle(ServerGame &, RemoteClient &) override;
	};
}
