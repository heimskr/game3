#pragma once

#include <array>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "game/ChunkPosition.h"

namespace Game3 {
	constexpr size_t CHUNK_SIZE = 64;
	constexpr size_t LAYER_COUNT = 3;

	using Chunk = std::vector<TileID>;

	class TileProvider {
		public:
			using ChunkMap = std::unordered_map<ChunkPosition, Chunk>;

			/** Behavior when accessing a tile in an out-of-bounds chunk.
			 *  The Create mode will cause a nonexistent chunk to be created on access. */
			enum class Mode {Throw, ReturnEmpty, Create};

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, bool &was_empty, Mode = Mode::Throw) const;

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, Mode = Mode::Throw) const;

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Index row, Index column, bool &created, Mode = Mode::Throw);

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Index row, Index column, Mode = Mode::Throw);

			const Chunk & getChunk(Layer, const ChunkPosition &) const;
			Chunk & getChunk(Layer, const ChunkPosition &);

			/** Creates missing chunks at a given chunk position in every layer. */
			void ensureChunk(const ChunkPosition &);

			/** Creates missing chunks at a given chunk position in every layer except the one provided (which should be 1-based). */
			void ensureChunk(const ChunkPosition &, Layer except);

		private:
			std::array<ChunkMap, LAYER_COUNT> chunkMaps {};

			void validateLayer(Layer) const;

			friend void to_json(nlohmann::json &, const TileProvider &);
			friend void from_json(const nlohmann::json &, TileProvider &);
	};

	void to_json(nlohmann::json &, const TileProvider &);
	void from_json(const nlohmann::json &, TileProvider &);
}
