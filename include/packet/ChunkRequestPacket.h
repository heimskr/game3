#pragma once

#include <set>

#include "game/ChunkPosition.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ChunkRequestPacket: Packet {
		static PacketID ID() { return 3; }

		RealmID realmID;
		std::set<ChunkRequest> requests;

		ChunkRequestPacket() = default;
		ChunkRequestPacket(RealmID realm_id, std::set<ChunkRequest> requests_):
			realmID(realm_id), requests(std::move(requests_)) {}
		ChunkRequestPacket(Realm &, const std::set<ChunkPosition> &, bool no_threshold = false);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override;
		void decode(Game &, Buffer &buffer)       override;

		void handle(ServerGame &, RemoteClient &) override;
	};
}
