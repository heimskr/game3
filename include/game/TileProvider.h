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

	class Game;
	class Tileset;

	template <typename T>
	using Chunk = std::vector<T>;

	class TileProvider {
		public:
			using ChunkMap = std::unordered_map<ChunkPosition, Chunk<TileID>>;
			using BiomeMap = std::unordered_map<ChunkPosition, Chunk<BiomeType>>;

			/** Behavior when accessing a tile in an out-of-bounds chunk.
			 *  The Create mode will cause a nonexistent chunk to be created on access. */
			enum class TileMode {Throw, ReturnEmpty, Create};
			enum class BiomeMode {Throw, Create};

			Identifier tilesetID;

			TileProvider() = default;

			void clear();

			std::shared_ptr<Tileset> getTileset(const Game &) const;

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, bool &was_empty, TileMode = TileMode::Throw) const;

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, TileMode = TileMode::Throw) const;

			/** Returns a copy of the biome type at a given tile position. */
			std::optional<BiomeType> copyBiomeType(Index row, Index column) const;

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Index row, Index column, bool &created, TileMode = TileMode::Throw);

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Index row, Index column, TileMode = TileMode::Throw);

			/** Returns a reference to the biome type at a given tile position. */
			BiomeType & findBiomeType(Index row, Index column, bool &created, BiomeMode = BiomeMode::Throw);

			/** Returns a reference to the biome type at a given tile position. */
			BiomeType & findBiomeType(Index row, Index column, BiomeMode = BiomeMode::Throw);

			const Chunk<TileID> & getTileChunk(Layer, const ChunkPosition &) const;
			Chunk<TileID> & getTileChunk(Layer, const ChunkPosition &);

			const Chunk<BiomeType> & getBiomeChunk(const ChunkPosition &) const;
			Chunk<BiomeType> & getBiomeChunk(const ChunkPosition &);

			/** Creates missing tile chunks at a given chunk position in every layer. */
			void ensureTileChunk(const ChunkPosition &);

			/** Creates missing tile chunks at a given chunk position in every layer except the one provided (which should be 1-based). */
			void ensureTileChunk(const ChunkPosition &, Layer except);

			/** Creates a missing biome chunk. */
			void ensureBiomeChunk(const ChunkPosition &);

		private:
			std::array<ChunkMap, LAYER_COUNT> chunkMaps {};
			BiomeMap biomeMap;

			void validateLayer(Layer) const;

			friend void to_json(nlohmann::json &, const TileProvider &);
			friend void from_json(const nlohmann::json &, TileProvider &);
	};

	void to_json(nlohmann::json &, const TileProvider &);
	void from_json(const nlohmann::json &, TileProvider &);

	ChunkPosition getChunkPosition(Index row, Index column);
}
