#include "data/WorldDB.h"
#include "game/Game.h"
#include "realm/Realm.h"

#include <bit>

namespace {
	template <typename S, typename T>
	void appendVector(S &raw, const std::vector<T> &source) {
		if constexpr (std::endian::native == std::endian::little) {
			const size_t start = raw.size();
			const size_t byte_count = source.size() * sizeof(source[0]);
			raw.resize(raw.size() + byte_count);
			std::memcpy(&raw[start], source.data(), byte_count);
		} else {
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
			appendVector(raw, chunk);
		}

		{
			std::shared_lock<std::shared_mutex> lock;
			if (!unsafe)
				lock = std::shared_lock(provider.biomeMutex);
			appendVector(raw, provider.biomeMap.at(chunk_position));
		}

		{
			std::shared_lock<std::shared_mutex> lock;
			if (!unsafe)
				lock = std::shared_lock(provider.fluidMutex);

			std::vector<uint32_t> raw_fluids;
			raw_fluids.reserve(CHUNK_SIZE * CHUNK_SIZE);

			for (const auto &tile: provider.fluidMap.at(chunk_position))
				raw_fluids.emplace_back(tile);

			appendVector(raw, raw_fluids);
		}

		const std::string key = getKey(realm, chunk_position);
		assert(database);
		database->Put(leveldb::WriteOptions(), key, raw);
	}

	std::string WorldDB::getKey(const std::shared_ptr<Realm> &realm, ChunkPosition chunk_position) {
		std::string out;
		appendBytes(out, realm->id);
		appendBytes(out, chunk_position.x);
		appendBytes(out, chunk_position.y);
		return out;
	}
}
