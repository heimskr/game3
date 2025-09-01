#include "data/GameDB.h"
#include "entity/Entity.h"
#include "entity/EntityFactory.h"
#include "error/FailedMigrationError.h"
#include "game/ServerGame.h"
#include "graphics/Tileset.h"
#include "net/Buffer.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "util/JSON.h"
#include "util/Timer.h"

#include <leveldb/write_batch.h>

namespace Game3 {
	namespace {
		const std::string CHUNK_PREFIX{"C::"};
		const std::string ENTITY_PREFIX{"E::"};
		const std::string META_PREFIX{"M::"};
		const std::string REALM_PREFIX{"R::"};
		const std::string TILE_ENTITY_PREFIX{"T::"};
		const std::string VILLAGE_PREFIX{"V::"};
		const std::string FORMAT_VERSION_KEY{META_PREFIX + "formatVersion"};
		const std::string GAME_RULES_KEY{META_PREFIX + "gameRules"};

		std::string getKey(const Village &village) {
			return std::format("{}{}", VILLAGE_PREFIX, village.getID());
		}

		std::string getKey(RealmID realm_id, ChunkPosition chunk_position) {
			return std::format("{}{}::{},{}", CHUNK_PREFIX, realm_id, chunk_position.x, chunk_position.y);
		}

		std::string getKey(RealmID realm_id) {
			return std::format("{}{}", REALM_PREFIX, realm_id);
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
			for (const auto &[position, tile_entity]: tile_entities) {
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
		std::string start = std::format("{}{}::", CHUNK_PREFIX, realm->getID());
		auto it = getStartIterator();

		for (; it->Valid(); it->Next()) {
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

	void GameDB::writeChunk(const RealmPtr &realm, ChunkPosition chunk_position) {
		Timer timer{"WriteChunk"};
		GameDBScope scope{*this};

		TileProvider &provider = realm->tileProvider;

		Buffer buffer{Side::Server,
			realm->getID(),
			chunk_position,
			Timer{"GetRawTerrain"}([&] { return provider.getRawTerrain(chunk_position); }),
			Timer{"GetRawBiomes" }([&] { return provider.getRawBiomes(chunk_position);  }),
			Timer{"GetRawFluids" }([&] { return provider.getRawFluids(chunk_position);  }),
			Timer{"GetRawPathmap"}([&] { return provider.getRawPathmap(chunk_position); }),
		};

		auto db_lock = database.uniqueLock();
		write(getKey(realm->getID(), chunk_position), buffer);
	}

	void GameDB::readAllRealms() {
		assert(database);
		ServerGamePtr game = getGame();
		auto db_lock = database.uniqueLock();

		iterate(CHUNK_PREFIX, [&](std::string_view, std::string_view value) {
			Timer iteration_timer{"ChunkLoad"};
			ViewBuffer buffer(value, Side::Server);

			RealmID realm_id;
			ChunkPosition chunk_position;
			std::span<const char> raw_terrain;
			std::span<const char> raw_biomes;
			std::span<const char> raw_fluids;
			std::span<const char> raw_pathmap;

			buffer >> realm_id >> chunk_position >> raw_terrain >> raw_biomes >> raw_fluids >> raw_pathmap;

			auto chunk_set = Timer{"ChunkSet"}([&] {
				return ChunkSet{
					raw_terrain,
					raw_biomes,
					raw_fluids,
					raw_pathmap,
				};
			});

			RealmPtr realm = Timer{"GetRealm"}([&] {
				return game->getRealm(realm_id, [&] {
					return loadRealm(realm_id);
				});
			});

			Timer{"Absorb"}([&] {
				realm->tileProvider.absorb(chunk_position, std::move(chunk_set));
			});
		});

		// If a realm has no chunks, it won't get loaded above, so we have to go through all realms
		// and load the ones that haven't been loaded yet.

		iterate(REALM_PREFIX, [&](std::string_view, std::string_view value) {
			const auto *start = reinterpret_cast<const uint8_t *>(value.data());
			size_t realm_id_size = 1 + sizeof(RealmID);
			assert(value.size() >= realm_id_size);
			std::vector<uint8_t> bytes(start, start + realm_id_size); // type indicator for RealmID is 1 byte in size
			RealmID realm_id = Buffer(std::move(bytes), Side::Server).take<RealmID>();
			game->getRealm(realm_id, [&] { return loadRealm(realm_id); });
		});

		Timer{"LoadVillages"}([&] {
			readVillages();
		});

		const bool force_migrate = std::filesystem::exists(".force-migrate");
		if (force_migrate) {
			std::filesystem::remove(".force-migrate");
		}

		std::unordered_map<std::string, boost::json::value> meta_cache;

		auto get_meta = [&](const std::string &hash) -> const boost::json::value & {
			if (auto iter = meta_cache.find(hash); iter != meta_cache.end()) {
				return iter->second;
			}
			boost::json::value json;
			readTilesetMeta(hash, json);
			return meta_cache[hash] = std::move(json);
		};

		game->iterateRealms([&](const RealmPtr &realm) {
			Tileset &tileset = realm->getTileset();

			const std::string realm_tileset_hash = readRealmTilesetHash(realm->id);
			if (tileset.getHash() == realm_tileset_hash) {
				return;
			}

			const boost::json::value &meta = get_meta(realm_tileset_hash);

			std::unordered_map<TileID, Identifier> old_map;
			try {
				old_map = boost::json::value_to<std::unordered_map<TileID, Identifier>>(meta.at("names"));
			} catch (const boost::system::system_error &) {
				old_map = loadKeyValuePairs<std::unordered_map, TileID, Identifier>(meta.at("names"));
			}

			std::unordered_map<Identifier, Identifier> autotiles;
			try {
				autotiles = boost::json::value_to<std::unordered_map<Identifier, Identifier>>(meta.at("autotiles"));
			} catch (const boost::system::system_error &) {
				autotiles = loadKeyValuePairs<std::unordered_map, Identifier, Identifier>(meta.at("autotiles"));
			}

			INFO("Auto-migrating tiles for realm {}", realm->id);

			Timer migration_timer{"TileMigration"};
			std::unordered_map<TileID, TileID> migration_map;

			const std::unordered_map<Identifier, TileID> new_map = tileset.getIDs();

			for (const auto &[numeric, identifier]: old_map) {
				if (auto new_tile_iter = new_map.find(identifier); new_tile_iter != new_map.end()) {
					migration_map[numeric] = new_tile_iter->second;
				}
			}

			TileProvider &provider = realm->tileProvider;
			std::unordered_set<TileID> covered;
			std::unordered_set<TileID> warned;

			for (Layer layer: allLayers) {
				std::unique_lock chunk_map_lock{provider.chunkMutexes.at(getIndex(layer))};
				TileProvider::ChunkMap &chunk_map = provider.chunkMaps.at(getIndex(layer));
				for (auto &[chunk_position, chunk]: chunk_map) {
					std::unique_lock chunk_lock = chunk.uniqueLock();
					for (TileID &tile_id: chunk) {
						const TileID old_tile = tile_id;
						if (auto iter = migration_map.find(tile_id); iter != migration_map.end()) {
							TileID new_tile = iter->second;

							tile_id = new_tile;
							if (new_tile != old_tile && covered.insert(old_tile).second) {
								INFO("{} ({}) → {} ({})", old_map.at(old_tile), old_tile, tileset.getNames().at(new_tile), new_tile);
							}
						} else if (force_migrate) {
							if (warned.insert(old_tile).second) {
								WARN("Replacing tile {} ({}) with nothing.", old_map.at(old_tile), old_tile);
							}
							tile_id = 0;
						} else {
							Identifier tilename = old_map.at(old_tile);
							ERR("Canceling tile migration; tile {} ({}) is missing from the new tileset. Create .force-migrate to force migration.", tilename, old_tile);
							throw FailedMigrationError("Migration failed due to missing tile " + tilename.str() + " (" + std::to_string(old_tile) + ')');
						}
					}
				}
			}

			for (Layer layer: allLayers) {
				std::vector<ChunkPosition> all_chunk_positions;
				{
					std::shared_lock chunk_map_lock{provider.chunkMutexes.at(getIndex(layer))};
					const TileProvider::ChunkMap &chunk_map = provider.chunkMaps.at(getIndex(layer));
					all_chunk_positions.reserve(chunk_map.size());
					for (const auto &[chunk_position, chunk]: chunk_map) {
						all_chunk_positions.emplace_back(chunk_position);
					}
				}

				for (ChunkPosition chunk_position: all_chunk_positions) {
					chunk_position.iterate([&](Position position) {
						realm->autotile(position, layer, TileUpdateContext{9});
					});
				}
			}

			SUCCESS("Finished tile migration for realm {}", realm->getID());
		});
	}

	RealmPtr GameDB::loadRealm(RealmID realm_id) {
		GameDBScope scope{*this};
		ServerGamePtr game = getGame();

		std::string raw;
		DBStatus status = database->Get(getReadOptions(), getKey(realm_id), &raw);

		if (!status) {
			throw std::out_of_range(std::format("Couldn't find realm {} in database", realm_id));
		}

		Buffer buffer(raw, Side::Server);

		assert(realm_id == buffer.take<RealmID>());
		auto json = buffer.take<boost::json::value>();
		auto tileset_hash = buffer.take<std::string>();

		RealmPtr realm = Realm::fromJSON(game, json, false);

		iterate(TILE_ENTITY_PREFIX, [&](std::string_view, std::string_view value) {
			ViewBuffer buffer{value, Side::Server};
			auto gid = buffer.take<GlobalID>();
			if (buffer.take<RealmID>() != realm_id) {
				return;
			}
			auto tile_entity_id = buffer.take<Identifier>();
			buffer.take<Position>();
			buffer.take<Identifier>(); // tileID
			auto factory = game->registry<TileEntityFactoryRegistry>().at(tile_entity_id);
			assert(factory);
			TileEntityPtr tile_entity = (*factory)();
			tile_entity->setGID(gid);
			tile_entity->setRealm(realm);
			buffer.context = game;
			tile_entity->init(*game);
			tile_entity->decode(*game, buffer);
			realm->addToMaps(tile_entity);
			realm->attach(tile_entity);
			tile_entity->onSpawn();
		});

		iterate(ENTITY_PREFIX, [&](std::string_view, std::string_view value) {
			ViewBuffer buffer{value, Side::Server};
			auto gid = buffer.take<GlobalID>();
			if (buffer.take<RealmID>() != realm_id) {
				return;
			}
			buffer.take<Position>();
			buffer.take<Direction>();
			auto entity_id = buffer.take<Identifier>();
			auto factory = game->registry<EntityFactoryRegistry>().at(entity_id);
			assert(factory);
			EntityPtr entity = (*factory)(game);
			entity->setGID(gid);
			entity->setRealm(realm);
			buffer.context = game;
			entity->decode(buffer);
			entity->init(game);
			{
				auto lock = realm->entities.uniqueLock();
				realm->entities.insert(entity);
			}
			{
				auto lock = realm->entitiesByGID.uniqueLock();
				realm->entitiesByGID[entity->globalID] = entity;
			}
			realm->attach(entity);
		});

		return realm;
	}

	void GameDB::writeRealmMeta(const RealmPtr &realm) {
		GameDBScope scope{*this};
		boost::json::value json;
		realm->toJSON(json, false);
		write(getKey(realm->id), Buffer{Side::Server,
			realm->id,
			json,
			realm->getTileset().getHash(),
		});
	}

	std::optional<ChunkSet> GameDB::getChunk(RealmID realm_id, ChunkPosition chunk_position) {
		GameDBScope scope{*this};

		std::optional<std::string> raw = tryRead(getKey(realm_id, chunk_position));
		if (!raw) {
			return std::nullopt;
		}

		ViewBuffer buffer(*raw, Side::Server);
		std::span<const char> raw_terrain, raw_biomes, raw_fluids, raw_pathmap;
		assert(realm_id == buffer.take<RealmID>());
		buffer.take<ChunkPosition>();
		buffer >> chunk_position >> raw_terrain >> raw_biomes >> raw_fluids >> raw_pathmap;

		return Timer{"ChunkSet"}([&] {
			return std::make_optional<ChunkSet>(raw_terrain, raw_biomes, raw_fluids, raw_pathmap);
		});
	}

	template <>
	std::optional<ChunkPosition> GameDB::tryRead<ChunkPosition>(const leveldb::Slice &key) {
		if (auto pair = tryReadNumbers<ChunkPosition::IntType, 2>(key)) {
			return std::make_optional<ChunkPosition>((*pair)[0], (*pair)[1]);
		}
		return std::nullopt;
	}

	template <>
	std::optional<Position> GameDB::tryRead<Position>(const leveldb::Slice &key) {
		if (auto pair = tryReadNumbers<Position::IntType, 2>(key)) {
			return std::make_optional<Position>((*pair)[0], (*pair)[1]);
		}
		return std::nullopt;
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

	std::unique_ptr<leveldb::Iterator> GameDB::getIterator() {
		return std::unique_ptr<leveldb::Iterator>(database->NewIterator(getReadOptions()));
	}

	std::unique_ptr<leveldb::Iterator> GameDB::getStartIterator() {
		auto it = getIterator();
		it->SeekToFirst();
		return it;
	}
}
