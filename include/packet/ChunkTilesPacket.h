#pragma once

#include "types/ChunkPosition.h"
#include "fluid/Fluid.h"
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
		std::vector<uint8_t> pathmap;

		ChunkTilesPacket() = default;
		ChunkTilesPacket(Realm &, ChunkPosition, uint64_t update_counter);
		ChunkTilesPacket(Realm &, ChunkPosition);
		ChunkTilesPacket(RealmID realm_id, ChunkPosition chunk_position, uint64_t update_counter, std::vector<TileID> tiles, std::vector<FluidTile> fluids, std::vector<uint8_t> pathmap):
			realmID(realm_id), chunkPosition(chunk_position), updateCounter(update_counter), tiles(std::move(tiles)), fluids(std::move(fluids)), pathmap(std::move(pathmap)) {}

		PacketID getID() const override { return ID(); }

		void encode(Game &, Buffer &buffer) const override;
		void decode(Game &, BasicBuffer &buffer)  override;

		void handle(const std::shared_ptr<ClientGame> &) override;
	};
}
