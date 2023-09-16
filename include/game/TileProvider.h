#pragma once

#include <array>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "Constants.h"
#include "Position.h"
#include "Types.h"
#include "data/ChunkSet.h"
#include "game/ChunkPosition.h"
#include "game/Fluids.h"
#include "threading/Lockable.h"
#include "threading/MTQueue.h"
#include "util/Math.h"

namespace Game3 {
	class Game;
	class Tileset;

	struct ChunkMeta {
		uint64_t updateCount = 0;
	};

	class TileProvider {
		public:
			using ChunkMap = std::unordered_map<ChunkPosition, TileChunk>;
			using BiomeMap = std::unordered_map<ChunkPosition, BiomeChunk>;
			using PathMap  = std::unordered_map<ChunkPosition, PathChunk>;
			using FluidMap = std::unordered_map<ChunkPosition, FluidChunk>;
			using MetaMap  = std::unordered_map<ChunkPosition, ChunkMeta>;

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

			mutable std::array<std::shared_mutex, LAYER_COUNT> chunkMutexes;
			mutable std::shared_mutex biomeMutex;
			mutable std::shared_mutex pathMutex;
			mutable std::shared_mutex fluidMutex;
			mutable std::shared_mutex metaMutex;

			TileProvider() = default;
			TileProvider(Identifier tileset_id);

			void clear();
			bool contains(ChunkPosition) const;

			uint64_t updateChunk(ChunkPosition);
			/** If the chunk position isn't present in the meta map, this returns 0 without adding the chunk position to the meta map. */
			uint64_t getUpdateCounter(ChunkPosition);
			void setUpdateCounter(ChunkPosition, uint64_t);

			/** Copies the data from a ChunkSet object into this TileProvider object's terrain, biome and fluid data, then remakes the path map.
			 *  Doesn't lock any of the ChunkSet object's mutexes. */
			void absorb(ChunkPosition, ChunkSet);

			std::shared_ptr<Tileset> getTileset(const Game &);

			std::vector<Position> getLand(const Game &, const ChunkRange &, Index right_pad, Index bottom_pad);

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Position, bool &was_empty, TileMode = TileMode::Throw) const;
			TileID copyTileUnsafe(Layer, Position, bool &was_empty, TileMode = TileMode::Throw) const;

			/** Returns a copy of the given tile. The Create mode will be treated as Throw. */
			TileID copyTile(Layer, Position, TileMode = TileMode::Throw) const;

			std::optional<TileID> tryTile(Layer layer, const Position &position) const;

			/** Returns a copy of the biome type at a given tile position. */
			std::optional<BiomeType> copyBiomeType(Position) const;

			/** Returns a copy of the path state at a given tile position. */
			std::optional<uint8_t> copyPathState(Position) const;

			/** Returns a copy of the fluid ID/amount at a given tile position. */
			std::optional<FluidTile> copyFluidTile(Position) const;

			ChunkSet getChunkSet(ChunkPosition) const;

			/** An empty vector indicates failure. */
			std::string getRawChunks(ChunkPosition) const;

			/** An empty vector indicates failure. */
			std::string getRawTerrain(ChunkPosition) const;

			/** An empty vector indicates failure. */
			std::string getRawBiomes(ChunkPosition) const;

			/** An empty vector indicates failure. */
			std::string getRawPathmap(ChunkPosition) const;

			/** An empty vector indicates failure. */
			std::string getRawFluids(ChunkPosition) const;

			/** Returns a reference to the given tile and sets up a shared lock. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Position, bool &created, std::shared_lock<std::shared_mutex> *lock_out, TileMode = TileMode::Create);

			/** Returns a reference to the given tile and sets up a unique lock. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer, Position, bool &created, std::unique_lock<std::shared_mutex> *lock_out, TileMode = TileMode::Create);

			template <typename T>
			/** Returns a reference to the given tile. The ReturnEmpty mode will be treated as Throw. */
			TileID & findTile(Layer layer, Position position, T *lock_out, TileMode mode = TileMode::Create) {
				bool created{};
				return findTile(layer, position, created, lock_out, mode);
			}

			/** Returns a reference to the biome type at a given tile position. */
			BiomeType & findBiomeType(Position, bool &created, std::shared_lock<std::shared_mutex> *lock_out, BiomeMode = BiomeMode::Create);

			/** Returns a reference to the biome type at a given tile position. */
			BiomeType & findBiomeType(Position, bool &created, std::unique_lock<std::shared_mutex> *lock_out, BiomeMode = BiomeMode::Create);

			/** Returns a reference to the biome type at a given tile position. */
			template <typename T>
			BiomeType & findBiomeType(Position position, T *lock_out, BiomeMode mode = BiomeMode::Create) {
				bool created{};
				return findBiomeType(position, created, lock_out, mode);
			}

			/** Returns a reference to the path state at a given tile position and sets up a shared lock. */
			uint8_t & findPathState(Position, bool &created, std::shared_lock<std::shared_mutex> *lock_out, PathMode = PathMode::Create);

			/** Returns a reference to the path state at a given tile position and sets up a unique lock. */
			uint8_t & findPathState(Position, bool &created, std::unique_lock<std::shared_mutex> *lock_out, PathMode = PathMode::Create);

			/** Returns a reference to the path state at a given tile position. */
			template <typename T>
			uint8_t & findPathState(Position position, T *lock_out, PathMode mode = PathMode::Create) {
				bool created{};
				return findPathState(position, created, lock_out, mode);
			}

			FluidTile & findFluid(Position, std::shared_lock<std::shared_mutex> *lock_out, FluidMode mode = FluidMode::Create);

			FluidTile & findFluid(Position, std::unique_lock<std::shared_mutex> *lock_out, FluidMode mode = FluidMode::Create);

			const TileChunk & getTileChunk(Layer, ChunkPosition) const;
			TileChunk & getTileChunk(Layer, ChunkPosition);

			std::optional<std::reference_wrapper<const TileChunk>> tryTileChunk(Layer, ChunkPosition) const;
			std::optional<std::reference_wrapper<TileChunk>> tryTileChunk(Layer, ChunkPosition);

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

			void ensureAllChunks(Position);

			void toJSON(nlohmann::json &, bool full_data = false) const;
			void absorbJSON(const nlohmann::json &, bool full_data = false);

			template <typename T>
			static T access(const Chunk<T> &chunk, int64_t row, int64_t column) {
				assert(0 <= row);
				assert(0 <= column);
				auto lock = chunk.sharedLock();
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
			MetaMap metaMap;

			void validateLayer(Layer) const;
			void initTileChunk(Layer, TileChunk &, ChunkPosition);
			void initBiomeChunk(Chunk<BiomeType> &, ChunkPosition);
			void initPathChunk(Chunk<uint8_t> &, ChunkPosition);
			void initFluidChunk(Chunk<FluidTile> &, ChunkPosition);
	};

	ChunkPosition getChunkPosition(Position);
}
