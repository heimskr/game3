#pragma once

#include "game/ChunkPosition.h"
#include "game/Fluids.h"
#include "net/Buffer.h"
#include "packet/Packet.h"

namespace Game3 {
	struct ChunkTilesPacket: Packet {
		static PacketID ID() { return 8; }

		RealmID realmID;
		ChunkPosition chunkPosition;
		uint64_t updateCounter = 0;
		std::vector<TileID> tiles;
		std::vector<FluidTile> fluids;

		ChunkTilesPacket() = default;
		ChunkTilesPacket(const Realm &, ChunkPosition, uint64_t update_counter);
		ChunkTilesPacket(Realm &, ChunkPosition);
		ChunkTilesPacket(RealmID realm_id, ChunkPosition chunk_position, uint64_t update_counter, std::vector<TileID> tiles_, std::vector<FluidTile> fluids_):
			realmID(realm_id), chunkPosition(chunk_position), updateCounter(update_counter), tiles(std::move(tiles_)), fluids(std::move(fluids_)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override { buffer << realmID << chunkPosition << updateCounter << tiles << fluids; }
		void decode(Game &, Buffer &buffer)       override { buffer >> realmID >> chunkPosition >> updateCounter >> tiles >> fluids; }

		void handle(ClientGame &) override;
	};
}
