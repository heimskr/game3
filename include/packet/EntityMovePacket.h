#pragma once

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct EntityMovePacket: Packet {
		static PacketID ID() { return 17; }

		GlobalID globalID = -1;
		RealmID realmID = -1;
		Position position;

		EntityMovePacket() = default;
		EntityMovePacket(const std::shared_ptr<Entity> &);
		EntityMovePacket(GlobalID global_id, RealmID realm_id, const Position &position_):
			globalID(global_id), realmID(realm_id), position(position_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << realmID << position; }
		void decode(Game &, Buffer &buffer)       override { buffer >> globalID >> realmID >> position; }

		void handle(ClientGame &) override;
	};
}
