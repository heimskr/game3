#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct EntitySetPathPacket: Packet {
		static PacketID ID() { return 19; }

		GlobalID globalID = -1;
		RealmID realmID = -1;
		Position position;
		std::vector<Direction> path;

		EntitySetPathPacket() = default;
		EntitySetPathPacket(Entity &);
		EntitySetPathPacket(GlobalID global_id, RealmID realm_id, const Position &position_, const std::vector<Direction> &path_):
			globalID(global_id), realmID(realm_id), position(position_), path(path_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << realmID << position << path; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> realmID >> position >> path; }

		void handle(ClientGame &) override;
	};
}
