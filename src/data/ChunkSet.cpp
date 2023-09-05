#include "data/ChunkSet.h"

namespace Game3 {
	ChunkSet::ChunkSet() {
		terrain.resize(LAYER_COUNT);
	}

	ChunkSet::ChunkSet(std::vector<TileChunk> terrain_, BiomeChunk biomes_, FluidChunk fluids_):
	terrain(std::move(terrain_)), biomes(std::move(biomes_)), fluids(std::move(fluids_)) {
		if (terrain.size() != LAYER_COUNT)
			throw std::invalid_argument("Invalid layer count in ChunkSet::ChunkSet: " + std::to_string(terrain.size()));
	}

	ChunkSet::ChunkSet(std::span<const char> raw):
		ChunkSet(std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(raw.data()), raw.size())) {}

	ChunkSet::ChunkSet(std::span<const uint8_t> raw) {
		constexpr size_t layer_byte_count  = CHUNK_SIZE * CHUNK_SIZE * sizeof(TileID);
		constexpr size_t biomes_byte_count = CHUNK_SIZE * CHUNK_SIZE * sizeof(BiomeType);
		constexpr size_t fluids_byte_count = CHUNK_SIZE * CHUNK_SIZE * sizeof(FluidInt);

		terrain.resize(LAYER_COUNT);

		for (TileChunk &layer: terrain) {
			// Can't use appendSpan because our span subtype is uint8_t but the data type we want to append is uint16_t.
			if constexpr (std::endian::native == std::endian::little) {
				layer.resize(CHUNK_SIZE * CHUNK_SIZE);
				std::memcpy(layer.data(), raw.data(), layer_byte_count);
			} else {
				layer.reserve(CHUNK_SIZE * CHUNK_SIZE);
				static_assert(sizeof(TileID) == 2);
				for (size_t i = 0; i < layer_byte_count; i += sizeof(TileID))
					layer.push_back(raw[i] | (TileID(raw[i + 1]) << 8));
			}

			raw = raw.subspan(layer_byte_count);
		}

		fluids.resize(CHUNK_SIZE * CHUNK_SIZE);

		if constexpr (std::endian::native == std::endian::little) {
			biomes.resize(CHUNK_SIZE * CHUNK_SIZE);
			std::memcpy(biomes.data(), raw.data(), biomes_byte_count);
			raw = raw.subspan(biomes_byte_count);
			static_assert(sizeof(FluidInt) == 4);
			for (size_t i = 0; i < fluids_byte_count; i += sizeof(FluidInt)) {
				FluidInt encoded_fluid{};
				std::memcpy(&encoded_fluid, &raw[i], sizeof(FluidInt));
				fluids.emplace_back(encoded_fluid);
			}
		} else {
			biomes.reserve(CHUNK_SIZE * CHUNK_SIZE);
			static_assert(sizeof(BiomeType) == 2);
			for (size_t i = 0; i < biomes_byte_count; i += sizeof(BiomeType))
				biomes.push_back(raw[i] | (BiomeType(raw[i + 1]) << 8));
			raw = raw.subspan(biomes_byte_count);
			static_assert(sizeof(FluidInt) == 4);
			for (size_t i = 0; i < fluids_byte_count; i += sizeof(FluidInt)) {
				const FluidInt encoded_fluid = raw[i] | (FluidInt(raw[i + 1]) << 8) | (FluidInt(raw[i + 2]) << 16) | (FluidInt(raw[i + 3]) << 24);
				fluids.emplace_back(encoded_fluid);
			}
		}
	}

	ChunkSet::FluidsArray ChunkSet::getFluids() const {
		FluidsArray out;
		assert(fluids.size() * sizeof(FluidInt) == out.size());

		auto lock = fluids.sharedLock();

		for (size_t i = 0, max = fluids.size(); i < max; ++i) {
			static_assert(sizeof(FluidInt) == 4);
			FluidInt encoded_fluid{fluids[i]};
			if constexpr (std::endian::native == std::endian::little) {
				std::memcpy(&out[i * sizeof(FluidInt)], &encoded_fluid, sizeof(FluidInt));
			} else {
				out[i] = encoded_fluid & 0xff;
				out[i + 1] = (encoded_fluid >>  8) & 0xff;
				out[i + 2] = (encoded_fluid >> 16) & 0xff;
				out[i + 3] = (encoded_fluid >> 24) & 0xff;
			}
		}

		return out;
	}
}
