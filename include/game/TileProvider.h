#pragma once

#include <array>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "Constants.h"
#include "Position.h"
#include "Types.h"
#include "game/ChunkPosition.h"
#include "game/Fluids.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"
#include "util/Math.h"

namespace Game3 {
	class Game;
	class Tileset;

	template <typename T>
	using Chunk = Lockable<std::vector<T>>;

	class TileProvider {
		public:
			using ChunkMap = std::unordered_map<ChunkPosition, Chunk<TileID>>;
			using BiomeMap = std::unordered_map<ChunkPosition, Chunk<BiomeType>>;
			using PathMap  = std::unordered_map<ChunkPosition, Chunk<uint8_t>>;
			using FluidMap = std::unordered_map<ChunkPosition, Chunk<FluidTile>>;

			/** Behavior when accessing a tile in an out-of-bounds chunk.
			 *  The Create mode will cause a nonexistent chunk to be created on access. */
			enum class TileMode  {Throw, Create, ReturnEmpty};
			enum class BiomeMode {Throw, Create};
			enum class PathMode  {Throw, Create};
			enum class FluidMode {Throw, Create};

			std::array<ChunkMap, LAYER_COUNT> chunkMaps;
			BiomeMap biomeMap;
			PathMap pathMap;
			FluidMap fluidMap;
			Identifier tilesetID;
			MTQueue<ChunkPosition> generationQueue;

			std::array<std::shared_mutex, LAYER_COUNT> chunkMutexes;
			std::shared_mutex biomeMutex;
			std::shared_mutex pathMutex;
			std::shared_mutex fluidMutex;

			TileProvider() = default;
			TileProvider(Identifier tileset_id);

			void clear();
			bool contains(ChunkPosition) const;

			std::shared_ptr<Tileset> getTileset(const Game &);

			std::vector<Position> getLand(const Game &, const ChunkRange &, Index right_pad, Index bottom_pad);

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Index row, Index column, bool &was_empty, TileMode = TileMode::Throw) const;
			TileID copyTileUnsafe(Layer, Index row, Index column, bool &was_empty, TileMode = TileMode::Throw) const;

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

			std::optional<TileID> tryTile(Layer layer, const Position &position) const;

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

			/** Returns a copy of the fluid ID/amount at a given tile position. */
			std::optional<FluidTile> copyFluidTile(Index row, Index column) const;

			/** Returns a copy of the fluid ID/amount at a given tile position. */
			inline std::optional<FluidTile> copyFluidTile(const Position &position) const {
				return copyFluidTile(position.row, position.column);
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

			const Chunk<TileID> & getTileChunk(Layer, ChunkPosition) const;
			Chunk<TileID> & getTileChunk(Layer, ChunkPosition);

			const Chunk<BiomeType> & getBiomeChunk(ChunkPosition) const;
			Chunk<BiomeType> & getBiomeChunk(ChunkPosition);

			const Chunk<uint8_t> & getPathChunk(ChunkPosition) const;
			Chunk<uint8_t> & getPathChunk(ChunkPosition);

			const Chunk<FluidTile> & getFluidChunk(ChunkPosition) const;
			Chunk<FluidTile> & getFluidChunk(ChunkPosition);

			/** Creates missing tile chunks at a given chunk position in every layer. */
			void ensureTileChunk(ChunkPosition);

			/** Creates missing tile chunks at a given chunk position in a given layer (which should be 1-based). */
			void ensureTileChunk(ChunkPosition, Layer);

			/** Creates a missing biome chunk. */
			void ensureBiomeChunk(ChunkPosition);

			/** Creates a missing path chunk. */
			void ensurePathChunk(ChunkPosition);

			/** Creates a missing fluid chunk. */
			void ensureFluidChunk(ChunkPosition);

			void ensureAllChunks(ChunkPosition);

			void ensureAllChunks(const Position &);

			template <typename T>
			static T access(const Chunk<T> &chunk, int64_t row, int64_t column) {
				assert(0 <= row);
				assert(0 <= column);
				auto lock = const_cast<Chunk<T> &>(chunk).sharedLock();
				return chunk[row * CHUNK_SIZE + column];
			}

			/** You need to lock the chunk yourself when using this method. */
			template <typename T>
			static T & access(Chunk<T> &chunk, int64_t row, int64_t column) {
				assert(0 <= row);
				assert(0 <= column);
				assert(chunk.data());
				return chunk[row * CHUNK_SIZE + column];
			}

			/** Given chunk_size = 32: [0, 32) -> 0, [32, 64) -> 1, [-32, 0) -> -1 */
			template <typename O = int32_t, typename I>
			static O divide(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
				if (0 <= value)
					return static_cast<O>(value / chunk_size);
				return static_cast<O>(-updiv(-value, chunk_size));
			}

			template <typename O = int32_t, typename I>
			static O remainder(I value, I chunk_size = static_cast<I>(CHUNK_SIZE)) {
				if (0 <= value)
					return static_cast<O>(value % chunk_size);
				return static_cast<O>((chunk_size - (-value % chunk_size)) % chunk_size);
			}

		private:
			std::shared_ptr<Tileset> cachedTileset;

			void validateLayer(Layer) const;
			void initTileChunk(Layer, Chunk<TileID> &, ChunkPosition);
			void initBiomeChunk(Chunk<BiomeType> &, ChunkPosition);
			void initPathChunk(Chunk<uint8_t> &, ChunkPosition);
			void initFluidChunk(Chunk<FluidTile> &, ChunkPosition);

			friend void to_json(nlohmann::json &, const TileProvider &);
			friend void from_json(const nlohmann::json &, TileProvider &);
	};

	void to_json(nlohmann::json &, const TileProvider &);
	void from_json(const nlohmann::json &, TileProvider &);

	ChunkPosition getChunkPosition(Index row, Index column);
	ChunkPosition getChunkPosition(const Position &);
}
