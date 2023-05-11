#pragma once

#include "Position.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	/** Indicates to the client that they've been teleported to a different realm. */
	struct SelfTeleportedPacket: Packet {
		static PacketID ID() { return 7; }

		RealmID realmID;
		Position position;

		SelfTeleportedPacket() = default;
		SelfTeleportedPacket(RealmID realm_id, const Position &position_):
			realmID(realm_id), position(position_) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << position; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID >> position; }
	};
}
