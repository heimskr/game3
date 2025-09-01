#pragma once

#include <set>

#include "types/ChunkPosition.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ChunkRequestPacket: Packet {
		static PacketID ID() { return 3; }

		RealmID realmID;
		std::set<ChunkRequest> requests;
		bool generateMissing = true;

		ChunkRequestPacket() = default;
		ChunkRequestPacket(RealmID realm_id, std::set<ChunkRequest> requests_, bool generate_missing = true):
			realmID(realm_id), requests(std::move(requests_)), generateMissing(generate_missing) {}
		ChunkRequestPacket(Realm &, const std::set<ChunkPosition> &, bool no_threshold = false, bool generate_missing = true);

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override;
		void decode(Game &, BasicBuffer &buffer)       override;

		void handle(const std::shared_ptr<ServerGame> &, GenericClient &) override;
	};
}
