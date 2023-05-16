#include "Log.h"
#include "game/ClientGame.h"
#include "game/TileProvider.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/PacketError.h"
#include "realm/Realm.h"

namespace Game3 {
	ChunkTilesPacket::ChunkTilesPacket(const Realm &realm, ChunkPosition chunk_position):
	realmID(realm.id), chunkPosition(chunk_position) {
		tiles.reserve(CHUNK_SIZE * CHUNK_SIZE * LAYER_COUNT);
		for (Layer layer = 1; layer <= LAYER_COUNT; ++layer) {
			const auto layer_tiles = realm.tileProvider.getTileChunk(1, chunk_position);
			tiles.insert(tiles.end(), layer_tiles.begin(), layer_tiles.end());
		}
	}

	void ChunkTilesPacket::handle(ClientGame &game) {
		if (tiles.size() != CHUNK_SIZE * CHUNK_SIZE * LAYER_COUNT)
			throw PacketError("Invalid tile count in ChunkTilesPacket: " + std::to_string(tiles.size()));
		auto realm = game.realms.at(realmID);
		auto &provider = realm->tileProvider;
		for (Layer layer = 1; layer <= LAYER_COUNT; ++layer) {
			auto &chunk = provider.getTileChunk(layer, chunkPosition);
			const size_t offset = (layer - 1) * CHUNK_SIZE * CHUNK_SIZE;
			chunk = std::vector<TileID>(tiles.begin() + offset, tiles.begin() + offset + CHUNK_SIZE * CHUNK_SIZE);
		}

		SUCCESS("Set tiles for chunk (" << chunkPosition.x << ", " << chunkPosition.y << ") in realm " << realmID);
	}
}
