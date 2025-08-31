#include "data/SqliteDB.h"
#include "entity/EntityFactory.h"
#include "entity/Player.h"
#include "error/FailedMigrationError.h"
#include "game/ServerGame.h"
#include "game/Village.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "util/Endian.h"
#include "util/JSON.h"
#include "util/Timer.h"
#include "util/Util.h"

#include <filesystem>
#include <iomanip>
#include <sstream>

namespace Game3 {
	SqliteDB::SqliteDB(const ServerGamePtr &game):
		weakGame(game) {}

	int64_t SqliteDB::getCurrentFormatVersion() {
		return 5;
	}

	std::string SqliteDB::getFileExtension() {
		return ".game3";
	}

	void SqliteDB::open(std::filesystem::path path_) {
		close();
		path = std::move(path_);
		auto db_lock = database.uniqueLock();
		database.getBase() = std::make_unique<SQLite::Database>(path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		static_assert(LAYER_COUNT * sizeof(TileID) * CHUNK_SIZE * CHUNK_SIZE < 131072);
		static_assert(sizeof(BiomeType) * CHUNK_SIZE * CHUNK_SIZE < 65536);
		static_assert(sizeof(FluidInt) * CHUNK_SIZE * CHUNK_SIZE < 65536);

		database->exec(R"(
			CREATE TABLE IF NOT EXISTS misc (
				key VARCHAR(64) PRIMARY KEY,
				value MEDIUMTEXT
			);

			CREATE TABLE IF NOT EXISTS chunks (
				realmID INT,
				x INT,
				y INT,
				terrain VARBINARY(131071),
				biomes  VARBINARY(65535),
				fluids  VARBINARY(65535),
				pathmap VARBINARY(65535),
				PRIMARY KEY (realmID, x, y)
			);

			CREATE TABLE IF NOT EXISTS rules (
				key VARCHAR(64) PRIMARY KEY,
				value INT8
			);

			CREATE TABLE IF NOT EXISTS users (
				username VARCHAR(32) PRIMARY KEY,
				displayName VARCHAR(64),
				encoded MEDIUMBLOB,
				releasePosition VARCHAR(42),
				releaseRealm INT
			);
		)" + Realm::getSQL() + TileEntity::getSQL() + Entity::getSQL() + Tileset::getSQL() + Village::getSQL());
	}

	void SqliteDB::close() {
		database.reset();
	}

	int64_t SqliteDB::getCompatibility() {
		assert(database != nullptr);
		auto lock = database.uniqueLock();
		SQLite::Statement query{*database, "SELECT value FROM misc WHERE key = ?"};
		query.bind(1, "formatVersion");
		if (query.executeStep()) {
			return parseNumber<int64_t>(query.getColumn(0).getString()) - getCurrentFormatVersion();
		}
		return INT64_MIN;
	}

	void SqliteDB::writeAll() {
		writeRules();
		writeAllRealms();
		ServerGamePtr game = getGame();
		auto player_lock = game->players.sharedLock();
		writeUsers(game->players);
		writeMisc();
	}

	void SqliteDB::readAll() {
		readRules();
		readAllRealms();
	}

	void SqliteDB::writeMisc() {
		auto lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO misc VALUES (?, ?)"};
		statement.bind(1, "formatVersion");
		statement.bind(2, getCurrentFormatVersion());
		statement.exec();
		statement.reset();
		transaction.commit();
	}

	void SqliteDB::writeRules() {
		ServerGamePtr game = getGame();
		auto rules_lock = game->gameRules.sharedLock();
		if (game->gameRules.empty()) {
			return;
		}
		auto lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO rules VALUES (?, ?)"};
		for (const auto &[key, value]: game->gameRules) {
			statement.bind(1, key);
			statement.bind(2, int64_t(value));
			statement.exec();
			statement.reset();
		}
		transaction.commit();
	}

	void SqliteDB::readRules() {
		ServerGamePtr game = getGame();
		auto lock = database.uniqueLock();
		auto rules_lock = game->gameRules.uniqueLock();

		SQLite::Statement query{*database, "SELECT * FROM rules"};
		while (query.executeStep()) {
			game->gameRules[query.getColumn(0)] = query.getColumn(1).getInt64();
		}
	}

	void SqliteDB::writeAllRealms() {
		Timer timer{"WriteAllRealms"};
		ServerGamePtr game = getGame();
		game->iterateRealms([this](const RealmPtr &realm) {
			writeRealm(realm);
		});
	}

	void SqliteDB::writeRealm(const RealmPtr &realm) {
		ServerGamePtr game = getGame();
		auto lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};

		{
			Timer timer{"WriteRealmMeta"};
			writeRealmMeta(realm, false);
		}
		{
			std::shared_lock lock(realm->tileProvider.chunkMutexes[0]);
			for (const auto &[chunk_position, chunk]: realm->tileProvider.chunkMaps[0]) {
				Timer timer{"WriteChunk"};
				writeChunk(realm, chunk_position, false);
			}
		}
		{
			Timer timer{"WriteTileEntities"};
			writeTileEntities(realm, false);
		}
		{
			Timer timer{"WriteEntities"};
			writeEntities(realm, false);
		}
		{
			Timer timer{"WriteTilesetMeta"};
			const Tileset &tileset = realm->getTileset();
			if (!hasTileset(tileset.getHash(), false)) {
				writeTilesetMeta(tileset, false);
			}
		}
		{
			Timer timer{"WriteVillages"};
			game->saveVillages(*database, false);
		}

		Timer timer{"WriteRealmCommit"};
		transaction.commit();
	}

	void SqliteDB::deleteRealm(RealmPtr realm) {
		assert(database);
		auto db_lock = database.uniqueLock();

		{
			auto lock = realm->tileEntities.sharedLock();
			for (const auto &[position, tile_entity]: realm->tileEntities) {
				deleteTileEntity(tile_entity);
			}
		}

		{
			auto lock = realm->entities.sharedLock();
			for (const EntityPtr &entity: realm->entities) {
				if (!entity->isPlayer()) {
					entity->onDestroy();
					deleteEntity(entity);
				}
			}
		}

		// Delete all the realm's chunks
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "DELETE FROM chunks WHERE realmID = ?"};
		statement.bind(1, std::make_signed_t<RealmID>(realm->getID()));
		statement.exec();
		transaction.commit();
	}

	void SqliteDB::writeChunk(const RealmPtr &realm, ChunkPosition chunk_position, bool use_transaction) {
		assert(database);
		TileProvider &provider = realm->tileProvider;

		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction) {
			transaction.emplace(*database);
		}

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO chunks VALUES (?, ?, ?, ?, ?, ?, ?)"};

		std::string raw_terrain;
		std::string raw_biomes;
		std::string raw_fluids;
		std::string raw_pathmap;

		{
			Timer timer{"GetRawTerrain"};
			raw_terrain = provider.getRawTerrain(chunk_position);
		}

		{
			Timer timer{"GetRawBiomes"};
			raw_biomes = provider.getRawBiomes(chunk_position);
		}

		{
			Timer timer{"GetRawFluids"};
			raw_fluids = provider.getRawFluids(chunk_position);
		}

		{
			Timer timer{"GetRawPathmap"};
			raw_pathmap = provider.getRawPathmap(chunk_position);
		}

		statement.bind(1, realm->id);
		statement.bind(2, chunk_position.x);
		statement.bind(3, chunk_position.y);
		{
			Timer timer{"BindTerrain"};
			statement.bind(4, raw_terrain);
		}
		{
			Timer timer{"BindBiomes"};
			statement.bind(5, raw_biomes);
		}
		{
			Timer timer{"BindFluids"};
			statement.bind(6, raw_fluids);
		}
		{
			Timer timer{"BindPathmap"};
			statement.bind(7, raw_pathmap);
		}

		{
			Timer timer{"ExecStatement"};
			statement.exec();
		}

		if (transaction) {
			Timer timer{"CommitTransaction"};
			transaction->commit();
		}
	}

	void SqliteDB::readAllRealms() {
		assert(database);
		ServerGamePtr game = getGame();
		auto db_lock = database.uniqueLock();

		SQLite::Statement chunk_query{*database, "SELECT * FROM chunks"};
		Timer chunk_query_timer{"ExecuteChunkStep"};

		while (chunk_query.executeStep()) {
			chunk_query_timer.stop();
			{
				Timer iteration_timer{"ChunkLoad"};
				RealmID realm_id = chunk_query.getColumn(0);
				ChunkPosition::IntType x = chunk_query.getColumn(1);
				ChunkPosition::IntType y = chunk_query.getColumn(2);
				const void *terrain = chunk_query.getColumn(3);
				const void *biomes  = chunk_query.getColumn(4);
				const void *fluids  = chunk_query.getColumn(5);
				const void *pathmap = chunk_query.getColumn(6);
				Timer chunk_set_timer{"ChunkSet"};
				ChunkSet chunk_set{
					std::span<const char>(reinterpret_cast<const char *>(terrain), chunk_query.getColumn(3).getBytes()),
					std::span<const char>(reinterpret_cast<const char *>(biomes),  chunk_query.getColumn(4).getBytes()),
					std::span<const char>(reinterpret_cast<const char *>(fluids),  chunk_query.getColumn(5).getBytes()),
					std::span<const char>(reinterpret_cast<const char *>(pathmap), chunk_query.getColumn(6).getBytes())
				};
				chunk_set_timer.stop();
				RealmPtr realm;
				{
					Timer get_realm_timer{"GetRealm"};
					realm = game->getRealm(realm_id, [&] { return loadRealm(realm_id, false); });
				}
				{
					Timer absorb_timer{"Absorb"};
					realm->tileProvider.absorb(ChunkPosition(x, y), std::move(chunk_set));
				}
			}
			chunk_query_timer.restart();
		}

		chunk_query_timer.stop();

		// If a realm has no chunks, it won't get loaded above, so we have to go through all realms
		// and load the ones that haven't been loaded yet.

		SQLite::Statement realm_query{*database, "SELECT realmID FROM realms"};
		Timer realm_query_timer{"ExecuteRealmStep"};

		while (realm_query.executeStep()) {
			realm_query_timer.stop();
			{
				Timer iteration_timer{"RealmLoad"};
				RealmID realm_id = realm_query.getColumn(0);
				game->getRealm(realm_id, [&] { return loadRealm(realm_id, false); });
			}
			realm_query_timer.restart();
		}

		realm_query_timer.stop();

		{
			Timer villages_timer{"LoadVillages"};
			game->loadVillages(game->toServerPointer(), *database);
		}

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
			readTilesetMeta(hash, json, false);
			return meta_cache[hash] = std::move(json);
		};

		game->iterateRealms([&](const RealmPtr &realm) {
			Tileset &tileset = realm->getTileset();

			const std::string realm_tileset_hash = readRealmTilesetHash(realm->id, false);
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

	RealmPtr SqliteDB::loadRealm(RealmID realm_id, bool do_lock) {
		assert(database);
		ServerGamePtr game = getGame();

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock) {
			db_lock = database.uniqueLock();
		}

		std::string raw_json;

		{
			SQLite::Statement query{*database, "SELECT json FROM realms WHERE realmID = ? LIMIT 1"};

			query.bind(1, realm_id);

			if (!query.executeStep()) {
				throw std::out_of_range("Couldn't find realm " + std::to_string(realm_id) + " in database");
			}

			raw_json = query.getColumn(0).getString();
		}

		RealmPtr realm = Realm::fromJSON(game, boost::json::parse(raw_json), false);

		{
			SQLite::Statement query{*database, "SELECT tileEntityID, encoded, globalID FROM tileEntities WHERE realmID = ?"};

			query.bind(1, realm_id);

			while (query.executeStep()) {
				const Identifier tile_entity_id{query.getColumn(0).getString()};
				auto factory = game->registry<TileEntityFactoryRegistry>().at(tile_entity_id);
				assert(factory);
				TileEntityPtr tile_entity = (*factory)();
				tile_entity->setGID(GlobalID(query.getColumn(2).getInt64()));

				const auto *buffer_bytes = reinterpret_cast<const uint8_t *>(query.getColumn(1).getBlob());
				const size_t buffer_size = query.getColumn(1).getBytes();

				tile_entity->setRealm(realm);

				Buffer buffer(std::vector<uint8_t>(buffer_bytes, buffer_bytes + buffer_size), Side::Server);
				buffer.context = game;
				tile_entity->init(*game);
				tile_entity->decode(*game, buffer);
				realm->addToMaps(tile_entity);
				realm->attach(tile_entity);
				tile_entity->onSpawn();
			}
		}

		{
			SQLite::Statement query{*database, "SELECT entityType, encoded FROM entities WHERE realmID = ?"};

			query.bind(1, realm_id);

			while (query.executeStep()) {
				const Identifier entity_id{query.getColumn(0).getString()};

				auto factory = game->registry<EntityFactoryRegistry>().at(entity_id);
				assert(factory);
				EntityPtr entity = (*factory)(game);

				const auto *buffer_bytes = reinterpret_cast<const uint8_t *>(query.getColumn(1).getBlob());
				const size_t buffer_size = query.getColumn(1).getBytes();

				entity->setRealm(realm);

				Buffer buffer(std::vector<uint8_t>(buffer_bytes, buffer_bytes + buffer_size), game, Side::Server);
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
			}
		}

		return realm;
	}

	void SqliteDB::writeRealmMeta(const RealmPtr &realm, bool use_transaction) {
		assert(database);

		boost::json::value json;
		realm->toJSON(json, false);

		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction) {
			transaction.emplace(*database);
		}

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO realms VALUES (?, ?, ?)"};

		statement.bind(1, realm->id);
		statement.bind(2, boost::json::serialize(json));
		statement.bind(3, realm->getTileset().getHash());

		statement.exec();

		if (transaction) {
			transaction->commit();
		}
	}

	std::optional<ChunkSet> SqliteDB::getChunk(RealmID realm_id, ChunkPosition chunk_position) {
		assert(database);

		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT terrain, biomes, fluids, pathmap FROM chunks WHERE realmID = ? AND x = ? AND y = ? LIMIT 1"};

		query.bind(1, realm_id);
		query.bind(2, chunk_position.x);
		query.bind(3, chunk_position.y);

		if (query.executeStep()) {
			const void *terrain = query.getColumn(0);
			const void *biomes  = query.getColumn(1);
			const void *fluids  = query.getColumn(2);
			const void *pathmap = query.getColumn(3);
			return ChunkSet{
				std::span<const char>(reinterpret_cast<const char *>(terrain), query.getColumn(0).getBytes()),
				std::span<const char>(reinterpret_cast<const char *>(biomes),  query.getColumn(1).getBytes()),
				std::span<const char>(reinterpret_cast<const char *>(fluids),  query.getColumn(2).getBytes()),
				std::span<const char>(reinterpret_cast<const char *>(pathmap), query.getColumn(3).getBytes())
			};
		}

		return std::nullopt;
	}

	bool SqliteDB::readUser(const std::string &username, std::string *display_name_out, Buffer *buffer_out, std::optional<Place> *release_place) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT displayName, encoded, releasePosition, releaseRealm FROM users WHERE username = ? LIMIT 1"};

		query.bind(1, username);

		if (query.executeStep()) {
			if (display_name_out != nullptr) {
				*display_name_out = std::string(query.getColumn(0));
			}

			if (buffer_out != nullptr) {
				SQLite::Column column = query.getColumn(1);
				const auto *blob = reinterpret_cast<const uint8_t *>(column.getBlob());
				*buffer_out = Buffer(std::vector<uint8_t>(blob, blob + column.getBytes()), getGame(), Side::Server);
			}

			if (release_place != nullptr) {
				if (query.isColumnNull(2) || query.isColumnNull(3)) {
					*release_place = std::nullopt;
				} else {
					ServerGamePtr game = getGame();
					*release_place = Place{Position{query.getColumn(2).getString()}, game->getRealm(query.getColumn(3).getInt()), nullptr};
				}
			}

			return true;
		}

		ERR("Couldn't read user \"{}\": {}", username, query.getErrorMsg());

		return false;
	}

	void SqliteDB::writeUser(const std::string &username, const std::string &display_name, const Buffer &buffer, const std::optional<Place> &release_place) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO users VALUES (?, ?, ?, ?, ?)"};

		statement.bind(1, username);
		statement.bind(2, display_name);
		statement.bind(3, buffer.bytes.data(), buffer.bytes.size());
		if (release_place) {
			statement.bind(4, release_place->position.simpleString());
			statement.bind(5, release_place->realm->id);
		} else {
			statement.bind(4);
			statement.bind(5);
		}

		statement.exec();
		transaction.commit();
	}

	void SqliteDB::writeUser(Player &player) {
		Buffer buffer{Side::Server};
		player.encode(buffer);
		writeUser(player.username.copyBase(), player.displayName, buffer, std::nullopt);
	}

	void SqliteDB::writeReleasePlace(const std::string &username, const std::optional<Place> &release_place) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "UPDATE users SET releasePosition = ?, releaseRealm = ? WHERE username = ?"};

		if (release_place) {
			statement.bind(1, release_place->position.simpleString());
			statement.bind(2, release_place->realm->id);
		} else {
			statement.bind(1);
			statement.bind(2);
		}
		statement.bind(3, username);

		statement.exec();
		transaction.commit();
	}

	bool SqliteDB::hasUsername(const std::string &username) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT NULL FROM users WHERE username = ? LIMIT 1"};

		query.bind(1, username);

		return query.executeStep();
	}

	bool SqliteDB::hasName(const std::string &username, const std::string &display_name) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT NULL FROM users WHERE username = ? OR displayName = ? LIMIT 1"};

		query.bind(1, username);
		query.bind(2, display_name);

		return query.executeStep();
	}

	std::optional<Place> SqliteDB::readReleasePlace(const std::string &username) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT releasePosition, releaseRealm FROM users WHERE username = ? LIMIT 1"};

		query.bind(1, username);

		if (query.executeStep()) {
			if (query.isColumnNull(0) || query.isColumnNull(1)) {
				return std::nullopt;
			}
			const std::string concatenated = query.getColumn(0).getString();
			if (concatenated.empty()) {
				return std::nullopt;
			}
			ServerGamePtr game = getGame();
			return Place{Position{concatenated}, game->getRealm(query.getColumn(1).getInt())};
		}

		return std::nullopt;
	}

	void SqliteDB::writeTileEntities(const std::function<bool(TileEntityPtr &)> &getter, bool use_transaction) {
		assert(database);
		TileEntityPtr tile_entity;
		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction) {
			transaction.emplace(*database);
		}

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO tileEntities VALUES (?, ?, ?, ?, ?, ?, ?)"};

		while (getter(tile_entity)) {
			statement.bind(1, std::make_signed_t<GlobalID>(tile_entity->getGID()));
			statement.bind(2, tile_entity->realmID);
			statement.bind(3, tile_entity->position.row);
			statement.bind(4, tile_entity->position.column);
			statement.bind(5, tile_entity->tileID.str());
			statement.bind(6, tile_entity->tileEntityID.str());
			Buffer buffer{Side::Server};
			tile_entity->encode(*tile_entity->getGame(), buffer);
			statement.bind(7, buffer.bytes.data(), buffer.bytes.size());
			statement.exec();
			statement.reset();
		}

		if (transaction) {
			transaction->commit();
		}
	}

	void SqliteDB::writeTileEntities(const RealmPtr &realm, bool use_transaction) {
		decltype(realm->tileEntities)::Base copy;
		{
			auto lock = realm->tileEntities.sharedLock();
			copy = realm->tileEntities.getBase();
		}
		auto iter = copy.begin();
		writeTileEntities([&](TileEntityPtr &out) {
			if (iter == copy.end()) {
				return false;
			}
			out = iter++->second;
			return true;
		}, use_transaction);
	}

	void SqliteDB::deleteTileEntity(const TileEntityPtr &tile_entity) {
		assert(database);
		auto db_lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "DELETE FROM tileEntities WHERE globalID = ?"};
		statement.bind(1, std::make_signed_t<GlobalID>(tile_entity->getGID()));
		statement.exec();
		transaction.commit();
	}

	void SqliteDB::writeEntities(const std::function<bool(EntityPtr &)> &getter, bool use_transaction) {
		assert(database);
		EntityPtr entity;
		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction) {
			transaction.emplace(*database);
		}

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO entities VALUES (?, ?, ?, ?, ?, ?, ?)"};

		while (getter(entity)) {
			if (!entity->shouldPersist() || entity->isPlayer()) {
				continue;
			}
			statement.bind(1, std::make_signed_t<GlobalID>(entity->getGID()));
			statement.bind(2, entity->realmID);
			statement.bind(3, entity->position.row);
			statement.bind(4, entity->position.column);
			statement.bind(5, entity->type.str());
			statement.bind(6, int(entity->direction.load()));
			Buffer buffer{Side::Server};
			entity->encode(buffer);
			statement.bind(7, buffer.bytes.data(), buffer.bytes.size());
			statement.exec();
			statement.reset();
		}

		if (transaction) {
			transaction->commit();
		}
	}

	void SqliteDB::writeEntities(const RealmPtr &realm, bool use_transaction) {
		decltype(realm->entities)::Base copy;
		{
			auto lock = realm->entities.sharedLock();
			copy = realm->entities.getBase();
		}
		auto iter = copy.begin();
		writeEntities([&](EntityPtr &out) {
			if (iter == copy.end()) {
				return false;
			}
			out = *iter++;
			return true;
		}, use_transaction);
	}

	void SqliteDB::deleteEntity(const EntityPtr &entity) {
		assert(database);
		auto db_lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "DELETE FROM entities WHERE globalID = ?"};
		statement.bind(1, std::make_signed_t<GlobalID>(entity->getGID()));
		statement.exec();
		transaction.commit();
	}

	std::string SqliteDB::readRealmTilesetHash(RealmID realm_id, bool do_lock) {
		assert(database);

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock) {
			db_lock = database.uniqueLock();
		}

		SQLite::Statement query{*database, "SELECT tilesetHash FROM realms WHERE realmID = ? LIMIT 1"};

		query.bind(1, realm_id);

		if (query.executeStep()) {
			return query.getColumn(0).getString();
		}

		throw std::out_of_range("Can't find tileset hash for realm " + std::to_string(realm_id));
	}

	bool SqliteDB::hasTileset(const std::string &hash, bool do_lock) {
		assert(database);

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock) {
			db_lock = database.uniqueLock();
		}

		SQLite::Statement query{*database, "SELECT NULL FROM tilesets WHERE hash = ? LIMIT 1"};

		query.bind(1, hash);

		return query.executeStep();
	}

	void SqliteDB::writeTilesetMeta(const Tileset &tileset, bool use_transaction) {
		assert(database);
		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction) {
			transaction.emplace(*database);
		}

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO tilesets VALUES (?, ?)"};

		boost::json::value json;
		tileset.getMeta(json);

		statement.bind(1, tileset.getHash());
		statement.bind(2, boost::json::serialize(json));

		statement.exec();

		if (transaction) {
			transaction->commit();
		}
	}

	bool SqliteDB::readTilesetMeta(const std::string &hash, boost::json::value &json, bool do_lock) {
		assert(database);

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock) {
			db_lock = database.uniqueLock();
		}

		SQLite::Statement query{*database, "SELECT json FROM tilesets WHERE hash = ? LIMIT 1"};

		query.bind(1, hash);

		if (query.executeStep()) {
			json = boost::json::parse(query.getColumn(0).getString());
			return true;
		}

		return false;
	}

	void SqliteDB::bind(SQLite::Statement &statement, const PlayerPtr &player) {
		statement.bind(1, player->username);
		statement.bind(2, player->displayName);
		Buffer buffer{Side::Server};
		player->encode(buffer);
		statement.bind(3, buffer.bytes.data(), buffer.bytes.size());
		statement.bind(4);
		statement.bind(5);
	}
}
