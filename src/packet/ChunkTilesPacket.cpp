#include "game/ClientGame.h"
#include "game/TileProvider.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/PacketError.h"

namespace Game3 {
	void ChunkTilesPacket::handle(ClientGame &game) const {
		if (tiles.size() != CHUNK_SIZE * CHUNK_SIZE * LAYER_COUNT)
			throw PacketError("Invalid tile count in ChunkTilesPacket: " + std::to_string(tiles.size()));
		auto realm = game.realms.at(realmID);
		auto &provider = realm->tileProvider;
		for (Layer layer = 1; layer <= LAYER_COUNT; ++layer) {
			auto &chunk = provider.getTileChunk(layer, chunkPosition);
			const size_t offset = (layer - 1) * CHUNK_SIZE * CHUNK_SIZE;
			chunk = std::vector<TileID>(tiles.begin() + offset, tiles.begin() + offset + CHUNK_SIZE * CHUNK_SIZE);
		}
	}
}
