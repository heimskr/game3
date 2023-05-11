#pragma once

#include "game/ChunkPosition.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ChunkRequestPacket: Packet {
		static PacketID ID() { return 3; }

		RealmID realmID;
		std::set<ChunkPosition> chunkPositions;

		ChunkRequestPacket() = default;
		ChunkRequestPacket(RealmID realm_id, std::set<ChunkPosition> chunk_positions):
			realmID(realm_id), chunkPositions(std::move(chunk_positions)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override;
		void decode(Game &, Buffer &buffer)       override;
	};
}
