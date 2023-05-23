#pragma once

#include <optional>

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct EntityMovePacket: Packet {
		static PacketID ID() { return 17; }

		GlobalID globalID = -1;
		RealmID realmID = -1;
		RealmID previousRealm = -1;
		Position position;
		Direction facing;
		std::optional<float> xOffset;
		std::optional<float> yOffset;

		EntityMovePacket() = default;
		EntityMovePacket(const Entity &);
		EntityMovePacket(GlobalID global_id, RealmID realm_id, RealmID previous_realm, const Position &position_, Direction facing_, std::optional<float> x_offset = {}, std::optional<float> y_offset = {}):
			globalID(global_id), realmID(realm_id), previousRealm(previous_realm), position(position_), facing(facing_), xOffset(x_offset), yOffset(y_offset) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << realmID << previousRealm << position << facing << xOffset << yOffset; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> realmID >> previousRealm >> position >> facing >> xOffset >> yOffset; }

		void handle(ClientGame &) override;
	};
}
