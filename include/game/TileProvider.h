#pragma once

#include <array>
#include <unordered_map>

#include "Types.h"
#include "game/ChunkPosition.h"

namespace Game3 {
	constexpr size_t CHUNK_SIZE = 32;
	constexpr size_t LAYER_COUNT = 3;

	struct Chunk: std::array<TileID, CHUNK_SIZE * CHUNK_SIZE> {
		using std::array<TileID, CHUNK_SIZE * CHUNK_SIZE>::array;
		TileID operator()(size_t row, size_t column) const;
		TileID & operator()(size_t row, size_t column);
	};

	class TileProvider {
		public:
			using ChunkMap = std::unordered_map<ChunkPosition, Chunk>;

			/** Behavior when accessing a tile in an out-of-bounds chunk.
			 *  The Create mode will cause a nonexistent chunk to be created on access. */
			enum class Mode {Throw, ReturnEmpty, Create};

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, Mode = Mode::Throw) const;

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Index row, Index column, Mode = Mode::Throw);

			const Chunk & getChunk(Layer, const ChunkPosition &) const;

			Chunk & getChunk(Layer, const ChunkPosition &);

		private:
			std::array<ChunkMap, LAYER_COUNT> chunkMaps {};

			void validateLayer(Layer) const;
	};
}
