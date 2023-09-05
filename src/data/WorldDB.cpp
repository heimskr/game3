#include "data/WorldDB.h"
#include "game/Game.h"
#include "realm/Realm.h"

#include <bit>

namespace {
	template <typename S, typename T>
	void appendSpan(S &raw, const std::span<T> &source) {
		const size_t byte_count = source.size() * sizeof(source[0]);
		if constexpr (std::endian::native == std::endian::little) {
			const size_t start = raw.size();
			raw.resize(raw.size() + byte_count);
			std::memcpy(&raw[start], source.data(), byte_count);
		} else {
			raw.reserve(raw.size() + byte_count);
			for (const auto item: source)
				for (size_t i = 0; i < sizeof(item); ++i)
					raw.push_back((item >> (8 * i)) & 0xff);
		}
	}

	template <typename S, typename T>
	void appendBytes(S &raw, T item) {
		for (size_t i = 0; i < sizeof(item); ++i)
			raw.push_back((item >> (8 * i)) & 0xff);
	}
}

namespace Game3 {
	ChunkSet::ChunkSet(const std::array<TileChunk, LAYER_COUNT> &terrain_, BiomeChunk biomes_, FluidChunk fluids_):
		terrain(terrain_), biomes(std::move(biomes_)), fluids(std::move(fluids_)) {}

	ChunkSet::ChunkSet(std::span<uint8_t> raw) {
		constexpr size_t layer_byte_count = CHUNK_SIZE * CHUNK_SIZE * sizeof(TileID);
		constexpr size_t biomes_byte_count = CHUNK_SIZE * CHUNK_SIZE * sizeof(BiomeType);
		constexpr size_t fluids_byte_count = CHUNK_SIZE * CHUNK_SIZE * sizeof(FluidInt);

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
		auto lock = sharedLock();

		assert(fluids.size() * sizeof(FluidInt) == out.size());

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

	WorldDB::WorldDB(Game &game_):
		game(game_) {}

	void WorldDB::open(std::filesystem::path path_) {
		close();
		path = std::move(path_);

		leveldb::DB *db;
		leveldb::Options options;
		options.create_if_missing = true;
		leveldb::Status status = leveldb::DB::Open(options, path, &db);
		database.reset(db);
	}

	void WorldDB::close() {
		database.reset();
	}

	void WorldDB::writeAll() {
		game.iterateRealms([this](const RealmPtr &realm) {
			std::vector<std::shared_lock<std::shared_mutex>> chunk_locks;
			chunk_locks.reserve(allLayers.size());
			for (auto &mutex: realm->tileProvider.chunkMutexes)
				chunk_locks.emplace_back(mutex);

			for (const auto &[chunk_position, chunk]: realm->tileProvider.chunkMaps[0])
				writeChunk(realm, chunk_position, true);
		});
	}

	void WorldDB::writeChunk(const RealmPtr &realm, ChunkPosition chunk_position, bool unsafe) {
		TileProvider &provider = realm->tileProvider;

		std::string raw;
		raw.reserve(LAYER_COUNT * CHUNK_SIZE * CHUNK_SIZE * sizeof(TileID));

		for (Layer layer: allLayers) {
			std::shared_lock<std::shared_mutex> lock;

			if (!unsafe)
				lock = std::shared_lock(provider.chunkMutexes[getIndex(layer)]);

			const TileChunk &chunk = provider.chunkMaps[getIndex(layer)].at(chunk_position);
			appendSpan(raw, std::span(chunk));
		}

		{
			std::shared_lock<std::shared_mutex> lock;
			if (!unsafe)
				lock = std::shared_lock(provider.biomeMutex);
			appendSpan(raw, std::span(provider.biomeMap.at(chunk_position)));
		}

		{
			std::shared_lock<std::shared_mutex> lock;
			if (!unsafe)
				lock = std::shared_lock(provider.fluidMutex);

			std::vector<uint32_t> raw_fluids;
			raw_fluids.reserve(CHUNK_SIZE * CHUNK_SIZE);

			for (const FluidTile &tile: provider.fluidMap.at(chunk_position))
				raw_fluids.emplace_back(tile);

			appendSpan(raw, std::span(raw_fluids));
		}

		const std::string key = getKey(realm->id, chunk_position);
		assert(database);
		database->Put({}, key, raw);
	}

	std::optional<TileChunk> WorldDB::getChunk(RealmID realm_id, ChunkPosition chunk_position) {
		std::string raw;
		if (!database->Get({}, getKey(realm_id, chunk_position), &raw).ok())
			return std::nullopt;

		// TODO!
	}

	std::string WorldDB::getKey(RealmID realm_id, ChunkPosition chunk_position) {
		std::string out;
		appendBytes(out, realm_id);
		appendBytes(out, chunk_position.x);
		appendBytes(out, chunk_position.y);
		return out;
	}
}
