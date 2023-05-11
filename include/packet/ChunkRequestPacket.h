#pragma once

#include "game/ChunkPosition.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ChunkRequestPacket: Packet {
		static PacketID ID() { return 3; }

		RealmID realmID;
		ChunkPosition chunkPosition;

		ChunkRequestPacket() = default;
		ChunkRequestPacket(RealmID realm_id, ChunkPosition chunk_position):
			realmID(realm_id), chunkPosition(chunk_position) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) override { buffer << realmID << chunkPosition; }
		void decode(Game &, Buffer &buffer) override { buffer >> realmID >> chunkPosition; }
	};
}
