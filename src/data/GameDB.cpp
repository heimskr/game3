#include "data/GameDB.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "net/Buffer.h"
#include "util/Timer.h"

#include <leveldb/write_batch.h>

namespace Game3 {
	namespace {
		const std::string FORMAT_VERSION_KEY{"M::formatVersion"};
		const std::string GAME_RULES_KEY{"M::gameRules"};

		std::string getKey(const Village &village) {
			return std::format("V::{}", village.getID());
		}

		std::string getKey(RealmID realm_id, ChunkPosition chunk_position) {
			return std::format("C::{}::{},{}", realm_id, chunk_position.x, chunk_position.y);
		}

		inline leveldb::Options getOpenOptions() {
			leveldb::Options options;
			options.create_if_missing = true;
			return options;
		}

		inline leveldb::ReadOptions getReadOptions() {
			leveldb::ReadOptions options;
			return options;
		}

		inline leveldb::WriteOptions getWriteOptions() {
			leveldb::WriteOptions options;
			return options;
		}
	}

	GameDBScope::GameDBScope(GameDB &game_db) {
		assert(game_db.database);
	}

	void DBStatus::assertOK() {
		if (!status.ok()) {
			throw DBError(status);
		}
	}

	GameDB::GameDB(const ServerGamePtr &game):
		weakGame(game) {}

	GameDB::~GameDB() {
		close();
	}

	int64_t GameDB::getCurrentFormatVersion() {
		return 6;
	}

	std::string GameDB::getFileExtension() {
		return ".game3";
	}

	void GameDB::open(std::filesystem::path path_) {
		close();
		path = std::move(path_);
		leveldb::DB *db = nullptr;
		leveldb::Options options = getOpenOptions();
		auto db_lock = database.uniqueLock();
		DBStatus status = leveldb::DB::Open(getOpenOptions(), path.string(), &db);
		status.assertOK();
		database.reset(db);
	}

	void GameDB::close() {
		auto db_lock = database.uniqueLock();
		database.reset();
	}

	template <>
	std::string GameDB::read<std::string>(const leveldb::Slice &key) {
		GameDBScope scope{*this};
		std::string out;
		DBStatus(database->Get(getReadOptions(), key, &out)).assertOK();
		return out;
	}

	void GameDB::write(const leveldb::Slice &key, const leveldb::Slice &value) {
		GameDBScope scope{*this};
		DBStatus(database->Put(getWriteOptions(), key, value)).assertOK();
	}

	void GameDB::erase(const leveldb::Slice &key) {
		GameDBScope scope{*this};
		DBStatus(database->Delete(getWriteOptions(), key)).assertOK();
	}

	int64_t GameDB::getCompatibility() {
		return readNumber<int64_t>(FORMAT_VERSION_KEY) - getCurrentFormatVersion();
	}

	void GameDB::writeAll() {
		writeRules();
		writeAllRealms();
		writeMisc();
		ServerGamePtr game = getGame();
		auto player_lock = game->players.sharedLock();
		writeUsers(game->players);
	}

	void GameDB::readAll() {
		readRules();
		readAllRealms();
	}

	void GameDB::writeMisc() {
		writeRaw(FORMAT_VERSION_KEY, getCurrentFormatVersion());
	}

	void GameDB::writeRules() {
		ServerGamePtr game = getGame();
		auto rules_lock = game->gameRules.sharedLock();
		if (game->gameRules.empty()) {
			erase(GAME_RULES_KEY);
		} else {
			write(GAME_RULES_KEY, boost::json::serialize(boost::json::value_from(game->gameRules.getBase())));
		}
	}

	void GameDB::readRules() {
		ServerGamePtr game = getGame();
		auto lock = database.uniqueLock();
		auto rules_lock = game->gameRules.uniqueLock();
		boost::json::parse_into(game->gameRules.getBase(), read(GAME_RULES_KEY));
	}

	void GameDB::writeAllRealms() {
		Timer timer{"WriteAllRealms"};
		getGame()->iterateRealms([this](const RealmPtr &realm) {
			writeRealm(realm);
		});
	}

	void GameDB::writeRealm(const RealmPtr &realm) {
		Timer timer{"WriteRealm"};
		ServerGamePtr game = getGame();
		auto lock = database.uniqueLock();
		writeRealmMeta(realm);
		{
			std::shared_lock lock(realm->tileProvider.chunkMutexes[0]);
			for (const auto &[chunk_position, chunk]: realm->tileProvider.chunkMaps[0]) {
				writeChunk(realm, chunk_position);
			}
		}
		writeTileEntities(realm);
		writeEntities(realm);
		if (const Tileset &tileset = realm->getTileset(); !hasTileset(tileset.getHash())) {
			writeTilesetMeta(tileset);
		}
		writeVillages();
	}

	void GameDB::deleteRealm(RealmPtr realm) {
		GameDBScope scope{*this};

		realm->tileEntities.withShared([&](const auto &tile_entities) {
			for (const auto &[position, tile_entity]: realm->tileEntities) {
				deleteTileEntity(tile_entity);
			}
		});

		realm->entities.withShared([&](const auto &entities) {
			for (const EntityPtr &entity: entities) {
				if (!entity->isPlayer()) {
					entity->onDestroy();
					deleteEntity(entity);
				}
			}
		});

		// Delete all chunks associated with the realm

		std::vector<std::string> keys;
		std::string start = std::format("C::{}::", realm->getID());
		std::unique_ptr<leveldb::Iterator> it(database->NewIterator(getReadOptions()));

		for (it->SeekToFirst(); it->Valid(); it->Next()) {
			if (it->key().starts_with(start)) {
				keys.emplace_back(it->key().ToString());
			}
		}

		DBStatus(it->status()).assertOK();
		it.reset();

		leveldb::WriteBatch batch;

		for (const std::string &key: keys) {
			batch.Delete(key);
		}

		DBStatus(database->Write(getWriteOptions(), &batch)).assertOK();
	}

	void GameDB::writeVillages() {
		Timer timer{"WriteVillages"};
		GamePtr game = getGame();
		const auto &village_map = game->getVillageMap();
		auto lock = village_map.sharedLock();
		for (const auto &[id, village]: village_map) {
			write(getKey(*village), Buffer{Side::Server, *village});
		}
	}

	template <>
	ChunkPosition GameDB::read<ChunkPosition>(const leveldb::Slice &key) {
		auto pair = readNumbers<ChunkPosition::IntType, 2>(key);
		return {pair[0], pair[1]};
	}

	template <>
	Position GameDB::read<Position>(const leveldb::Slice &key) {
		auto pair = readNumbers<Position::IntType, 2>(key);
		return {pair[0], pair[1]};
	}

	void GameDB::write(const leveldb::Slice &key, ChunkPosition value) {
		writeNumbers(key, std::array{value.x, value.y});
	}

	void GameDB::write(const leveldb::Slice &key, Position value) {
		if constexpr (Position::ROW_FIRST) {
			writeNumbers(key, std::array{value.row, value.column});
		} else {
			writeNumbers(key, std::array{value.column, value.row});
		}
	}

	void GameDB::write(const leveldb::Slice &key, const Buffer &buffer) {
		std::span<const uint8_t> span = buffer.getSpan();
		leveldb::Slice slice(reinterpret_cast<const char *>(span.data()), span.size_bytes());
		write(key, slice);
	}
}
