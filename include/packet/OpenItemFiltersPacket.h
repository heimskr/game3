#pragma once

#include "net/Buffer.h"
#include "packet/Packet.h"
#include "types/Direction.h"
#include "types/Position.h"

namespace Game3 {
	struct OpenItemFiltersPacket: Packet {
		static PacketID ID() { return 26; }

		RealmID realmID;
		Position position;
		Direction direction;
		bool removeOnMove = true;

		OpenItemFiltersPacket() = default;
		OpenItemFiltersPacket(RealmID realm_id, const Position &position, Direction direction, bool remove_on_move = true):
			realmID(realm_id), position(position), direction(direction), removeOnMove(remove_on_move) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << position << direction << removeOnMove; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> realmID >> position >> direction >> removeOnMove; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
