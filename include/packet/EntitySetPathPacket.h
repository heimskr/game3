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
		UpdateCounter newCounter = -1;

		EntitySetPathPacket() = default;
		EntitySetPathPacket(Entity &);
		EntitySetPathPacket(GlobalID global_id, RealmID realm_id, const Position &position_, std::vector<Direction> path_, UpdateCounter new_counter):
			globalID(global_id), realmID(realm_id), position(position_), path(std::move(path_)), newCounter(new_counter) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << realmID << position << path << newCounter; }
		void decode(Game &, BasicBuffer &buffer)       override { buffer >> globalID >> realmID >> position >> path >> newCounter; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
