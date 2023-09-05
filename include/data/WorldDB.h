#pragma once

#include "Layer.h"
#include "Types.h"
#include "game/Chunk.h"
#include "game/ChunkPosition.h"
#include "game/Fluids.h"
#include "threading/HasMutex.h"

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <leveldb/db.h>

namespace Game3 {
	class Game;
	class Realm;

	class ChunkSet: public HasMutex<> {
		private:
			using FluidsArray = std::array<uint8_t, CHUNK_SIZE * CHUNK_SIZE * sizeof(FluidInt)>;
			FluidsArray getFluids() const;

		public:
			std::array<TileChunk, LAYER_COUNT> terrain{};
			BiomeChunk biomes{};
			FluidChunk fluids{};

			ChunkSet() = default;
			ChunkSet(const std::array<TileChunk, LAYER_COUNT> &, BiomeChunk, FluidChunk);
			ChunkSet(std::span<uint8_t>);

			template <typename C>
			C getBytes() const {
				// Format:
				//   - Terrain: [ChunkSize^2 x sizeof(TileID) bytes] x LayerCount
				//   - Biomes:   ChunkSize^2 x sizeof(BiomeType)
				//   - Fluids:   ChunkSize^2 x sizeof(FluidInt)
				// Or, as of this writing:
				//   - Terrain: [4096 x 2 bytes] x 4 = 32768 bytes
				//   - Biomes:   4096 x 2 bytes      =  8192 bytes
				//   - Fluids:   4096 x 4 bytes      = 16384 bytes
				//   Total: 57344 bytes
				C raw;
				constexpr size_t square = CHUNK_SIZE * CHUNK_SIZE;
				raw.reserve((LAYER_COUNT * sizeof(TileID) + sizeof(BiomeType) + sizeof(FluidInt)) * square);

				{
					auto lock = sharedLock();

					for (const TileChunk &chunk: terrain)
						appendSpan(raw, std::span(chunk));

					appendSpan(raw, std::span(biomes));
					appendSpan(raw, std::span(getFluids()));
				}

				return raw;
			}
	};

	class WorldDB {
		private:
			Game &game;
			std::unique_ptr<leveldb::DB> database;
			std::filesystem::path path;

			std::string getKey(RealmID, ChunkPosition);

		public:
			WorldDB(Game &);

			void open(std::filesystem::path);
			void close();

			void writeAll();
			void writeChunk(const std::shared_ptr<Realm> &, ChunkPosition, bool unsafe = false);
			std::optional<TileChunk> getChunk(RealmID, ChunkPosition);

			inline bool isOpen() {
				return database != nullptr;
			}
	};
}
