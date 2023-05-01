#pragma once

#include <array>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "Constants.h"
#include "Types.h"
#include "game/ChunkPosition.h"
#include "util/Math.h"

namespace Game3 {
	class Game;
	class Tileset;

	template <typename T>
	using Chunk = std::vector<T>;

	class TileProvider {
		public:
			using ChunkMap = std::unordered_map<ChunkPosition, Chunk<TileID>>;
			using BiomeMap = std::unordered_map<ChunkPosition, Chunk<BiomeType>>;
			using PathMap  = std::unordered_map<ChunkPosition, Chunk<uint8_t>>;

			/** Behavior when accessing a tile in an out-of-bounds chunk.
			 *  The Create mode will cause a nonexistent chunk to be created on access. */
			enum class TileMode  {Throw, Create, ReturnEmpty};
			enum class BiomeMode {Throw, Create};
			enum class PathMode  {Throw, Create};

			std::array<ChunkMap, LAYER_COUNT> chunkMaps;
			BiomeMap biomeMap;
			PathMap pathMap;
			Identifier tilesetID;

			TileProvider() = default;
			TileProvider(Identifier tileset_id);

			void clear();

			std::shared_ptr<Tileset> getTileset(const Game &);

			std::vector<Position> getLand(const Game &, const ChunkRange &, Index right_pad, Index bottom_pad);

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, bool &was_empty, TileMode = TileMode::Throw) const;

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, TileMode = TileMode::Throw) const;

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			inline TileID copyTile(Layer layer, const Position &position, bool &was_empty, TileMode mode = TileMode::Throw) const {
				return copyTile(layer, position.row, position.column, was_empty, mode);
			}

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			inline TileID copyTile(Layer layer, const Position &position, TileMode mode = TileMode::Throw) const {
				return copyTile(layer, position.row, position.column, mode);
			}

			/** Returns a copy of the biome type at a given tile position. */
			std::optional<BiomeType> copyBiomeType(Index row, Index column) const;

			/** Returns a copy of the biome type at a given tile position. */
			inline std::optional<BiomeType> copyBiomeType(const Position &position) const {
				return copyBiomeType(position.row, position.column);
			}

			/** Returns a copy of the path state at a given tile position. */
			std::optional<uint8_t> copyPathState(Index row, Index column) const;

			/** Returns a copy of the path state at a given tile position. */
			inline std::optional<uint8_t> copyPathState(const Position &position) const {
				return copyPathState(position.row, position.column);
			}

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Index row, Index column, bool &created, TileMode = TileMode::Create);

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			inline TileID & findTile(Layer layer, const Position &position, bool &created, TileMode mode = TileMode::Create) {
				return findTile(layer, position.row, position.column, created, mode);
			}

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Index row, Index column, TileMode = TileMode::Create);

			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			inline TileID & findTile(Layer layer, const Position &position, TileMode mode = TileMode::Create) {
				return findTile(layer, position.row, position.column, mode);
			}

			/** Returns a reference to the biome type at a given tile position. */
			BiomeType & findBiomeType(Index row, Index column, bool &created, BiomeMode = BiomeMode::Create);

			/** Returns a reference to the biome type at a given tile position. */
			inline BiomeType & findBiomeType(const Position &position, bool &created, BiomeMode mode = BiomeMode::Create) {
				return findBiomeType(position.row, position.column, created, mode);
			}

			/** Returns a reference to the biome type at a given tile position. */
			BiomeType & findBiomeType(Index row, Index column, BiomeMode = BiomeMode::Create);

			/** Returns a reference to the biome type at a given tile position. */
			inline BiomeType & findBiomeType(const Position &position, BiomeMode mode = BiomeMode::Create) {
				return findBiomeType(position.row, position.column, mode);
			}

			/** Returns a reference to the path state at a given tile position. */
			uint8_t & findPathState(Index row, Index column, bool &created, PathMode = PathMode::Create);

			/** Returns a reference to the path state at a given tile position. */
			inline uint8_t & findPathState(const Position &position, bool &created, PathMode mode = PathMode::Create) {
				return findPathState(position.row, position.column, created, mode);
			}

			/** Returns a reference to the path state at a given tile position. */
			uint8_t & findPathState(Index row, Index column, PathMode = PathMode::Create);

			/** Returns a reference to the path state at a given tile position. */
			inline uint8_t & findPathState(const Position &position, PathMode mode = PathMode::Create) {
				return findPathState(position.row, position.column, mode);
			}

			const Chunk<TileID> & getTileChunk(Layer, const ChunkPosition &) const;
			Chunk<TileID> & getTileChunk(Layer, const ChunkPosition &);

			const Chunk<BiomeType> & getBiomeChunk(const ChunkPosition &) const;
			Chunk<BiomeType> & getBiomeChunk(const ChunkPosition &);

			const Chunk<uint8_t> & getPathChunk(const ChunkPosition &) const;
			Chunk<uint8_t> & getPathChunk(const ChunkPosition &);

			/** Creates missing tile chunks at a given chunk position in every layer. */
			void ensureTileChunk(const ChunkPosition &);

			/** Creates missing tile chunks at a given chunk position in every layer except the one provided (which should be 1-based). */
			void ensureTileChunk(const ChunkPosition &, Layer except);

			/** Creates a missing biome chunk. */
			void ensureBiomeChunk(const ChunkPosition &);

			/** Creates a missing path chunk. */
			void ensurePathChunk(const ChunkPosition &);

			void ensureAllChunks(const ChunkPosition &);

			void ensureAllChunks(const Position &);

			template <typename T>
			static T access(const Chunk<T> &chunk, size_t row, size_t column) {
				return chunk[row * CHUNK_SIZE + column];
			}

			template <typename T>
			static T & access(Chunk<T> &chunk, size_t row, size_t column) {
				return chunk[row * CHUNK_SIZE + column];
			}

			/** Given chunk_size = 32: [0, 32) -> 0, [32, 64) -> 1, [-32, 0) -> -1 */
			template <typename I, typename O = int32_t>
			static O divide(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
				if (0 <= value)
					return static_cast<O>(value / chunk_size);
				return static_cast<O>(-updiv(-value, chunk_size));
			}

			template <typename I, typename O = int32_t>
			static O remainder(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
				if (0 <= value)
					return static_cast<O>(value % chunk_size);
				return static_cast<O>((chunk_size - (-value % chunk_size)) % chunk_size);
			}

		private:
			std::shared_ptr<Tileset> cachedTileset;

			void validateLayer(Layer) const;
			void initTileChunk(Layer, Chunk<TileID> &, const ChunkPosition &);
			void initBiomeChunk(Chunk<BiomeType> &, const ChunkPosition &);
			void initPathChunk(Chunk<uint8_t> &, const ChunkPosition &);

			friend void to_json(nlohmann::json &, const TileProvider &);
			friend void from_json(const nlohmann::json &, TileProvider &);
	};

	void to_json(nlohmann::json &, const TileProvider &);
	void from_json(const nlohmann::json &, TileProvider &);

	ChunkPosition getChunkPosition(Index row, Index column);
	ChunkPosition getChunkPosition(const Position &);
}
