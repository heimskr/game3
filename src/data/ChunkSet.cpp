// #include "Constants.h"
#include "Log.h"
#include "data/ChunkSet.h"
#include "game/Fluids.h"

#include <bit>
#include <cstddef>
#include <cstring>
#include <span>
#include <string>
#include <vector>

namespace Game3 {
	ChunkSet::ChunkSet() {
		terrain.resize(LAYER_COUNT);
	}

	ChunkSet::ChunkSet(std::vector<TileChunk> terrain_, BiomeChunk biomes_, FluidChunk fluids_, PathChunk pathmap_):
	terrain(std::move(terrain_)), biomes(std::move(biomes_)), fluids(std::move(fluids_)), pathmap(std::move(pathmap_)) {
		if (terrain.size() != LAYER_COUNT)
			throw std::invalid_argument("Invalid layer count in ChunkSet::ChunkSet: " + std::to_string(terrain.size()));
	}

	ChunkSet::ChunkSet(std::span<const uint8_t> raw):
		ChunkSet(std::span<const char>(reinterpret_cast<const char *>(raw.data()), raw.size())) {}

	constexpr size_t LAYER_BYTE_COUNT   = CHUNK_SIZE * CHUNK_SIZE * sizeof(TileID);
	constexpr size_t BIOMES_BYTE_COUNT  = CHUNK_SIZE * CHUNK_SIZE * sizeof(BiomeType);
	constexpr size_t FLUIDS_BYTE_COUNT  = CHUNK_SIZE * CHUNK_SIZE * sizeof(FluidInt);
	constexpr size_t PATHMAP_BYTE_COUNT = CHUNK_SIZE * CHUNK_SIZE * sizeof(uint8_t);

	ChunkSet::ChunkSet(std::span<const char> raw):
		ChunkSet(raw.subspan(0, LAYER_COUNT * LAYER_BYTE_COUNT),
		         raw.subspan(LAYER_COUNT * LAYER_BYTE_COUNT, BIOMES_BYTE_COUNT),
		         raw.subspan(LAYER_COUNT * LAYER_BYTE_COUNT + BIOMES_BYTE_COUNT, FLUIDS_BYTE_COUNT),
				 raw.subspan(LAYER_COUNT * LAYER_BYTE_COUNT + BIOMES_BYTE_COUNT + FLUIDS_BYTE_COUNT, PATHMAP_BYTE_COUNT)) {}

	ChunkSet::ChunkSet(std::span<const char> terrain_, std::span<const char> biomes_, std::span<const char> fluids_, std::span<const char> pathmap_) {
		terrain.resize(LAYER_COUNT);

		for (TileChunk &layer: terrain) {
			// Can't use appendSpan because our span subtype is char but the data type we want to append is uint16_t.
			if constexpr (std::endian::native == std::endian::little) {
				layer.resize(CHUNK_SIZE * CHUNK_SIZE);
				std::memcpy(layer.data(), terrain_.data(), LAYER_BYTE_COUNT);
			} else {
				layer.reserve(CHUNK_SIZE * CHUNK_SIZE);
				static_assert(sizeof(TileID) == 2);
				for (size_t i = 0; i < LAYER_BYTE_COUNT; i += sizeof(TileID))
					layer.emplace_back(terrain_[i] | (TileID(terrain_[i + 1]) << 8));
			}

			terrain_ = terrain_.subspan(LAYER_BYTE_COUNT);
		}

		if constexpr (std::endian::native == std::endian::little) {
			biomes.resize(CHUNK_SIZE * CHUNK_SIZE);
			std::memcpy(biomes.data(), biomes_.data(), BIOMES_BYTE_COUNT);

			fluids.reserve(CHUNK_SIZE * CHUNK_SIZE);
			for (size_t i = 0; i < FLUIDS_BYTE_COUNT; i += sizeof(FluidInt)) {
				FluidInt encoded_fluid{};
				std::memcpy(&encoded_fluid, &fluids_[i], sizeof(FluidInt));
				fluids.emplace_back(encoded_fluid);
			}
			assert(fluids.size() == CHUNK_SIZE * CHUNK_SIZE);
		} else {
			biomes.reserve(CHUNK_SIZE * CHUNK_SIZE);
			static_assert(sizeof(BiomeType) == 2);
			for (size_t i = 0; i < BIOMES_BYTE_COUNT; i += sizeof(BiomeType))
				biomes.emplace_back(biomes_[i] | (BiomeType(biomes_[i + 1]) << 8));

			fluids.reserve(CHUNK_SIZE * CHUNK_SIZE);
			static_assert(sizeof(FluidInt) == 8);
			for (size_t i = 0; i < FLUIDS_BYTE_COUNT; i += sizeof(FluidInt)) {
				const FluidInt encoded_fluid =
					fluids_[i] | (FluidInt(fluids_[i + 1]) << 8) | (FluidInt(fluids_[i + 2]) << 16) | (FluidInt(fluids_[i + 3]) << 24)
					| (FluidInt(fluids_[i + 4]) << 32) | (FluidInt(fluids_[i + 5]) << 40) | (FluidInt(fluids_[i + 6]) << 48)| (FluidInt(fluids_[i + 7]) << 56);
				fluids.emplace_back(encoded_fluid);
			}
			assert(fluids.size() == CHUNK_SIZE * CHUNK_SIZE);
		}

		static_assert(sizeof(decltype(pathmap)::value_type) == 1);
		pathmap.resize(CHUNK_SIZE * CHUNK_SIZE);
		std::memcpy(pathmap.data(), pathmap_.data(), PATHMAP_BYTE_COUNT);
	}

	ChunkSet::FluidsArray ChunkSet::getFluids() const {
		FluidsArray out;
		assert(fluids.size() * sizeof(FluidInt) == out.size());

		auto lock = fluids.sharedLock();

		for (size_t i = 0, max = fluids.size(); i < max; ++i) {
			static_assert(sizeof(FluidInt) == 8);
			FluidInt encoded_fluid{fluids[i]};
			if constexpr (std::endian::native == std::endian::little) {
				std::memcpy(&out[i * sizeof(FluidInt)], &encoded_fluid, sizeof(FluidInt));
			} else {
				out[i] = encoded_fluid & 0xff;
				out[i + 1] = (encoded_fluid >>  8) & 0xff;
				out[i + 2] = (encoded_fluid >> 16) & 0xff;
				out[i + 3] = (encoded_fluid >> 24) & 0xff;
				out[i + 4] = (encoded_fluid >> 32) & 0xff;
				out[i + 5] = (encoded_fluid >> 40) & 0xff;
				out[i + 6] = (encoded_fluid >> 48) & 0xff;
				out[i + 7] = (encoded_fluid >> 56) & 0xff;
			}
		}

		return out;
	}
}
