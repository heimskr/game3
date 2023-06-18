#pragma once

#include <optional>

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct EntityMovedPacket: Packet {
		static PacketID ID() { return 17; }

		GlobalID globalID = -1;
		RealmID realmID = -1;
		Position position;
		Direction facing;
		std::optional<Offset> offset;
		std::optional<float> zSpeed;

		EntityMovedPacket() = default;
		EntityMovedPacket(const Entity &);
		EntityMovedPacket(GlobalID global_id, RealmID realm_id, const Position &position_, Direction facing_, std::optional<Offset> offset_ = {}, std::optional<float> z_speed = {}):
			globalID(global_id), realmID(realm_id), position(position_), facing(facing_), offset(offset_), zSpeed(z_speed) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << realmID << position << facing << offset << zSpeed; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> realmID >> position >> facing >> offset >> zSpeed; }

		void handle(ClientGame &) override;
	};
}
