#include "Log.h"
#include "game/ClientGame.h"
#include "game/TileProvider.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/PacketError.h"
#include "realm/Realm.h"

namespace Game3 {
	ChunkTilesPacket::ChunkTilesPacket(const Realm &realm, ChunkPosition chunk_position, uint64_t update_counter):
	realmID(realm.id), chunkPosition(chunk_position), updateCounter(update_counter) {
		tiles.reserve(CHUNK_SIZE * CHUNK_SIZE * LAYER_COUNT);
		for (const auto layer: allLayers) {
			auto &layer_tiles = realm.tileProvider.getTileChunk(layer, chunk_position);
			auto lock = layer_tiles.sharedLock();
			tiles.insert(tiles.end(), layer_tiles.begin(), layer_tiles.end());
		}

		auto &fluid_chunk = realm.tileProvider.getFluidChunk(chunk_position);
		auto lock = fluid_chunk.sharedLock();
		fluids = fluid_chunk;
	}

	ChunkTilesPacket::ChunkTilesPacket(Realm &realm, ChunkPosition chunk_position): ChunkTilesPacket(realm, chunk_position, 0) {
		updateCounter = realm.tileProvider.getUpdateCounter(chunk_position);
	}


	void ChunkTilesPacket::handle(ClientGame &game) {
		if (tiles.size() != CHUNK_SIZE * CHUNK_SIZE * LAYER_COUNT)
			throw PacketError("Invalid tile count in ChunkTilesPacket: " + std::to_string(tiles.size()));

		if (fluids.size() != CHUNK_SIZE * CHUNK_SIZE)
			throw PacketError("Invalid fluid count in ChunkTilesPacket: " + std::to_string(fluids.size()));

		auto realm = game.realms.at(realmID);
		auto &provider = realm->tileProvider;
		for (const auto layer: allLayers) {
			auto &chunk = provider.getTileChunk(layer, chunkPosition);
			const size_t offset = getIndex(layer) * CHUNK_SIZE * CHUNK_SIZE;
			chunk = std::vector<TileID>(tiles.begin() + offset, tiles.begin() + offset + CHUNK_SIZE * CHUNK_SIZE);
		}

		provider.getFluidChunk(chunkPosition) = fluids;

		provider.setUpdateCounter(chunkPosition, updateCounter);

		game.chunkReceived(chunkPosition);
		realm->reupload();
		realm->reuploadFluids();
	}
}
