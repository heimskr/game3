#pragma once

#include "Constants.h"
#include "Layer.h"
#include "game/Fluids.h"
#include "threading/HasMutex.h"

#include <array>
#include <span>
#include <vector>

namespace Game3 {
	class ChunkSet {
		private:
			using FluidsArray = std::array<uint8_t, CHUNK_SIZE * CHUNK_SIZE * sizeof(FluidInt)>;
			FluidsArray getFluids() const;

		public:
			std::vector<TileChunk> terrain;
			BiomeChunk biomes;
			FluidChunk fluids;

			ChunkSet();
			ChunkSet(std::vector<TileChunk>, BiomeChunk, FluidChunk);
			ChunkSet(std::span<const uint8_t>);
			ChunkSet(std::span<const char>);
			ChunkSet(std::span<const char> terrain_, std::span<const char> biomes_, std::span<const char> fluids_);

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

				for (const TileChunk &chunk: terrain) {
					auto lock = chunk.sharedLock();
					appendSpan(raw, std::span(chunk));
				}

				{
					auto lock = biomes.sharedLock();
					appendSpan(raw, std::span(biomes));
				}

				auto fluids = getFluids();
				appendSpan(raw, std::span<const uint8_t>(fluids.data(), fluids.size()));
				return raw;
			}
	};
}
