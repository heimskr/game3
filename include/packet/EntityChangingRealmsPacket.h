#pragma once

#include <optional>

#include "game/Game.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	class Entity;

	struct EntityChangingRealmsPacket: Packet {
		static PacketID ID() { return 46; }

		GlobalID globalID = -1;
		RealmID newRealmID = -1;
		Position newPosition;

		EntityChangingRealmsPacket() = default;
		EntityChangingRealmsPacket(GlobalID global_id, RealmID new_realm_id, const Position &new_position):
			globalID(global_id), newRealmID(new_realm_id), newPosition(new_position) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << globalID << newRealmID << newPosition; }
		void decode(Game &, BasicBuffer &buffer)  override { buffer >> globalID >> newRealmID >> newPosition; }

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
