#include "data/GameDB.h"
#include "entity/EntityFactory.h"
#include "entity/Player.h"
#include "error/FailedMigrationError.h"
#include "game/ServerGame.h"
#include "game/Village.h"
#include "graphics/Tileset.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "util/Endian.h"
#include "util/Timer.h"
#include "util/Util.h"

#include <filesystem>
#include <iomanip>
#include <sstream>

#include <nlohmann/json.hpp>

namespace Game3 {
	GameDB::GameDB(ServerGame &game_):
		game(game_) {}

	void GameDB::open(std::filesystem::path path_) {
		close();
		path = std::move(path_);
		auto db_lock = database.uniqueLock();
		database.getBase() = std::make_unique<SQLite::Database>(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		static_assert(LAYER_COUNT * sizeof(TileID) * CHUNK_SIZE * CHUNK_SIZE < 65536);
		static_assert(sizeof(BiomeType) * CHUNK_SIZE * CHUNK_SIZE < 65536);
		static_assert(sizeof(FluidInt) * CHUNK_SIZE * CHUNK_SIZE < 65536);

		// std::p

		database->exec(R"(
			CREATE TABLE IF NOT EXISTS chunks (
				realmID INT,
				x INT,
				y INT,
				terrain VARBINARY(65535),
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
				json MEDIUMTEXT,
				releasePosition VARCHAR(42),
				releaseRealm INT
			);
		)" + Realm::getSQL() + TileEntity::getSQL() + Entity::getSQL() + Tileset::getSQL() + Village::getSQL());
	}

	void GameDB::close() {
		database.reset();
	}

	void GameDB::writeAll() {
		writeRules();
		writeAllRealms();
		auto player_lock = game.players.sharedLock();
		writeUsers(game.players);
	}

	void GameDB::readAll() {
		readRules();
		readAllRealms();
	}

	void GameDB::writeRules() {
		auto rules_lock = game.gameRules.sharedLock();
		if (game.gameRules.empty())
			return;
		auto lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO rules VALUES (?, ?)"};
		for (const auto &[key, value]: game.gameRules) {
			statement.bind(1, key);
			statement.bind(2, int64_t(value));
			statement.exec();
			statement.reset();
		}
		transaction.commit();
	}

	void GameDB::readRules() {
		auto lock = database.uniqueLock();
		auto rules_lock = game.gameRules.uniqueLock();

		SQLite::Statement query{*database, "SELECT * FROM rules"};
		while (query.executeStep()) {
			game.gameRules[query.getColumn(0)] = query.getColumn(1).getInt64();
		}
	}

	void GameDB::writeAllRealms() {
		Timer timer{"WriteAllRealms"};
		game.iterateRealms([this](const RealmPtr &realm) {
			writeRealm(realm);
		});
	}

	void GameDB::writeRealm(const RealmPtr &realm) {
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
			if (!hasTileset(tileset.getHash(), false))
				writeTilesetMeta(tileset, false);
		}
		{
			Timer timer{"WriteVillages"};
			game.saveVillages(*database, false);
		}

		Timer timer{"WriteRealmCommit"};
		transaction.commit();
	}

	void GameDB::writeChunk(const RealmPtr &realm, ChunkPosition chunk_position, bool use_transaction) {
		assert(database);
		TileProvider &provider = realm->tileProvider;

		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction)
			transaction.emplace(*database);

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

	void GameDB::readAllRealms() {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT * FROM chunks"};

		Timer query_timer{"ExecuteStep"};

		while (query.executeStep()) {
			query_timer.stop();
			{
				Timer iteration_timer{"ChunkLoad"};
				RealmID realm_id = query.getColumn(0);
				ChunkPosition::IntType x = query.getColumn(1);
				ChunkPosition::IntType y = query.getColumn(2);
				const void *terrain = query.getColumn(3);
				const void *biomes  = query.getColumn(4);
				const void *fluids  = query.getColumn(5);
				const void *pathmap = query.getColumn(6);
				Timer chunk_set_timer{"ChunkSet"};
				ChunkSet chunk_set{
					std::span<const char>(reinterpret_cast<const char *>(terrain), query.getColumn(3).getBytes()),
					std::span<const char>(reinterpret_cast<const char *>(biomes),  query.getColumn(4).getBytes()),
					std::span<const char>(reinterpret_cast<const char *>(fluids),  query.getColumn(5).getBytes()),
					std::span<const char>(reinterpret_cast<const char *>(pathmap), query.getColumn(6).getBytes())
				};
				chunk_set_timer.stop();
				RealmPtr realm;
				{
					Timer get_realm_timer{"GetRealm"};
					realm = game.getRealm(realm_id, [&] { return loadRealm(realm_id, false); });
				}
				{
					Timer absorb_timer{"Absorb"};
					realm->tileProvider.absorb(ChunkPosition(x, y), std::move(chunk_set));
				}
			}
			query_timer.restart();
		}

		{
			Timer villages_timer{"LoadVillages"};
			game.loadVillages(game.toServerPointer(), *database);
		}

		const bool force_migrate = std::filesystem::exists(".force-migrate");
		if (force_migrate)
			std::filesystem::remove(".force-migrate");

		std::unordered_map<std::string, nlohmann::json> meta_cache;

		auto get_meta = [&](const std::string &hash) -> const nlohmann::json & {
			if (auto iter = meta_cache.find(hash); iter != meta_cache.end())
				return iter->second;
			nlohmann::json json;
			readTilesetMeta(hash, json, false);
			return meta_cache[hash] = std::move(json);
		};

		game.iterateRealms([&](const RealmPtr &realm) {
			Tileset &tileset = realm->getTileset();

			const std::string realm_tileset_hash = readRealmTilesetHash(realm->id, false);
			if (tileset.getHash() == realm_tileset_hash)
				return;

			const nlohmann::json &meta = get_meta(realm_tileset_hash);
			const std::unordered_map<TileID, Identifier> old_map = meta.at("names");
			const std::unordered_map<Identifier, Identifier> autotiles = meta.at("autotiles");

			INFO_("Auto-migrating tiles for realm " << realm->id);

			Timer migration_timer{"TileMigration"};
			std::unordered_map<TileID, TileID> migration_map;

			const std::unordered_map<Identifier, TileID> new_map = tileset.getIDs();

			for (const auto &[numeric, identifier]: old_map)
				if (auto new_tile_iter = new_map.find(identifier); new_tile_iter != new_map.end())
					migration_map[numeric] = new_tile_iter->second;

			TileProvider &provider = realm->tileProvider;
			std::unordered_set<TileID> covered;
			std::unordered_set<TileID> warned;

			for (const Layer layer: allLayers) {
				std::unique_lock chunk_map_lock{provider.chunkMutexes.at(getIndex(layer))};
				TileProvider::ChunkMap &chunk_map = provider.chunkMaps.at(getIndex(layer));
				for (auto &[chunk_position, chunk]: chunk_map) {
					std::unique_lock chunk_lock = chunk.uniqueLock();
					for (TileID &tile_id: chunk) {
						const TileID old_tile = tile_id;
						if (auto iter = migration_map.find(tile_id); iter != migration_map.end()) {
							TileID new_tile = iter->second;

							// We don't want to automigrate old autotiles to the base of the new autotile.
							// Take the autotile offset of the old autotile and add it to the new autotile.
							if (autotiles.contains(old_map.at(old_tile)))
								new_tile += old_tile % 16;

							tile_id = new_tile;
							if (new_tile != old_tile && covered.insert(old_tile).second)
								INFO("{} ({}) → {} ({})", old_map.at(old_tile), old_tile, tileset.getNames().at(new_tile), new_tile);
						} else if (force_migrate) {
							if (warned.insert(old_tile).second)
								WARN("Replacing tile {} ({}) with nothing.", old_map.at(old_tile), old_tile);
							tile_id = 0;
						} else {
							Identifier tilename = old_map.at(old_tile);
							ERROR("Canceling tile migration; tile {} ({}) is missing from the new tileset. Create .force-migrate to force migration.", tilename, old_tile);
							throw FailedMigrationError("Migration failed due to missing tile " + tilename.str() + " (" + std::to_string(old_tile) + ')');
						}
					}
				}
			}

			SUCCESS("Finished tile migration for realm {}", realm->getID());
		});
	}

	RealmPtr GameDB::loadRealm(RealmID realm_id, bool do_lock) {
		assert(database);

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock)
			db_lock = database.uniqueLock();

		std::string raw_json;

		{
			SQLite::Statement query{*database, "SELECT json FROM realms WHERE realmID = ? LIMIT 1"};

			query.bind(1, realm_id);

			if (!query.executeStep())
				throw std::out_of_range("Couldn't find realm " + std::to_string(realm_id) + " in database");

			raw_json = query.getColumn(0).getString();
		}

		RealmPtr realm = Realm::fromJSON(game, nlohmann::json::parse(raw_json), false);

		{
			SQLite::Statement query{*database, "SELECT tileEntityID, encoded, globalID FROM tileEntities WHERE realmID = ?"};

			query.bind(1, realm_id);

			while (query.executeStep()) {
				const Identifier tile_entity_id{query.getColumn(0).getString()};
				auto factory = game.registry<TileEntityFactoryRegistry>().at(tile_entity_id);
				assert(factory);
				TileEntityPtr tile_entity = (*factory)();
				tile_entity->setGID(GlobalID(query.getColumn(2).getInt64()));

				const auto *buffer_bytes = reinterpret_cast<const uint8_t *>(query.getColumn(1).getBlob());
				const size_t buffer_size = query.getColumn(1).getBytes();

				tile_entity->setRealm(realm);

				Buffer buffer(std::vector<uint8_t>(buffer_bytes, buffer_bytes + buffer_size));
				buffer.context = game.shared_from_this();
				tile_entity->init(game);
				tile_entity->decode(game, buffer);

				// TODO: functionize this
				{
					auto lock = realm->tileEntities.uniqueLock();
					realm->tileEntities.emplace(tile_entity->position, tile_entity);
				}

				{
					auto lock = realm->tileEntitiesByGID.uniqueLock();
					realm->tileEntitiesByGID[tile_entity->globalID] = tile_entity;
				}

				realm->attach(tile_entity);
				tile_entity->onSpawn();
			}
		}

		{
			SQLite::Statement query{*database, "SELECT entityType, encoded FROM entities WHERE realmID = ?"};

			query.bind(1, realm_id);

			while (query.executeStep()) {
				const Identifier entity_id{query.getColumn(0).getString()};

				auto factory = game.registry<EntityFactoryRegistry>().at(entity_id);
				assert(factory);
				EntityPtr entity = (*factory)(game);

				const auto *buffer_bytes = reinterpret_cast<const uint8_t *>(query.getColumn(1).getBlob());
				const size_t buffer_size = query.getColumn(1).getBytes();

				entity->setRealm(realm);

				Buffer buffer(std::vector<uint8_t>(buffer_bytes, buffer_bytes + buffer_size));
				buffer.context = game.shared_from_this();
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

	void GameDB::writeRealmMeta(const RealmPtr &realm, bool use_transaction) {
		assert(database);

		nlohmann::json json;
		realm->toJSON(json, false);

		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction)
			transaction.emplace(*database);

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO realms VALUES (?, ?, ?)"};

		statement.bind(1, realm->id);
		statement.bind(2, json.dump());
		statement.bind(3, realm->getTileset().getHash());

		statement.exec();

		if (transaction)
			transaction->commit();
	}

	std::optional<ChunkSet> GameDB::getChunk(RealmID realm_id, ChunkPosition chunk_position) {
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

	bool GameDB::readUser(const std::string &username, std::string *display_name_out, nlohmann::json *json_out, std::optional<Place> *release_place) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT displayName, json, releasePosition, releaseRealm FROM users WHERE username = ? LIMIT 1"};

		query.bind(1, username);

		if (query.executeStep()) {
			if (display_name_out != nullptr)
				*display_name_out = std::string(query.getColumn(0));

			if (json_out != nullptr)
				*json_out = nlohmann::json::parse(std::string(query.getColumn(1)));

			if (release_place != nullptr) {
				if (query.isColumnNull(2) || query.isColumnNull(3))
					*release_place = std::nullopt;
				else
					*release_place = Place{Position{query.getColumn(2).getString()}, game.getRealm(query.getColumn(3).getInt()), nullptr};
			}

			return true;
		}

		return false;
	}

	void GameDB::writeUser(const std::string &username, const nlohmann::json &json, const std::optional<Place> &release_place) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO users VALUES (?, ?, ?, ?, ?)"};

		statement.bind(1, username);
		statement.bind(2, json.at("displayName").get<std::string>());
		statement.bind(3, json.dump());
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

	void GameDB::writeUser(const Player &player) {
		nlohmann::json json;
		player.toJSON(json);
		writeUser(player.username.copyBase(), json, std::nullopt);
	}

	void GameDB::writeReleasePlace(const std::string &username, const std::optional<Place> &release_place) {
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

	bool GameDB::hasName(const std::string &username, const std::string &display_name) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT NULL FROM users WHERE username = ? OR displayName = ? LIMIT 1"};

		query.bind(1, username);
		query.bind(2, display_name);

		return query.executeStep();
	}

	std::optional<Place> GameDB::readReleasePlace(const std::string &username) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT releasePosition, releaseRealm FROM users WHERE username = ? LIMIT 1"};

		query.bind(1, username);

		if (query.executeStep()) {
			if (query.isColumnNull(0) || query.isColumnNull(1))
				return std::nullopt;
			const std::string concatenated = query.getColumn(0).getString();
			if (concatenated.empty())
				return std::nullopt;
			return Place{Position{concatenated}, game.getRealm(query.getColumn(1).getInt())};
		}

		return std::nullopt;
	}

	void GameDB::writeTileEntities(const std::function<bool(TileEntityPtr &)> &getter, bool use_transaction) {
		assert(database);
		TileEntityPtr tile_entity;
		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction)
			transaction.emplace(*database);

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO tileEntities VALUES (?, ?, ?, ?, ?, ?, ?)"};

		while (getter(tile_entity)) {
			statement.bind(1, std::make_signed_t<GlobalID>(tile_entity->getGID()));
			statement.bind(2, tile_entity->realmID);
			statement.bind(3, tile_entity->position.row);
			statement.bind(4, tile_entity->position.column);
			statement.bind(5, tile_entity->tileID.str());
			statement.bind(6, tile_entity->tileEntityID.str());
			Buffer buffer;
			tile_entity->encode(tile_entity->getGame(), buffer);
			statement.bind(7, buffer.bytes.data(), buffer.bytes.size());
			statement.exec();
			statement.reset();
		}

		if (transaction)
			transaction->commit();
	}

	void GameDB::writeTileEntities(const RealmPtr &realm, bool use_transaction) {
		decltype(realm->tileEntities)::Base copy;
		{
			auto lock = realm->tileEntities.sharedLock();
			copy = realm->tileEntities.getBase();
		}
		auto iter = copy.begin();
		writeTileEntities([&](TileEntityPtr &out) {
			if (iter == copy.end())
				return false;
			out = iter++->second;
			return true;
		}, use_transaction);
	}

	void GameDB::deleteTileEntity(const TileEntityPtr &tile_entity) {
		assert(database);
		auto db_lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "DELETE FROM tileEntities WHERE globalID = ?"};
		statement.bind(1, std::make_signed_t<GlobalID>(tile_entity->getGID()));
		statement.exec();
		transaction.commit();
	}

	void GameDB::writeEntities(const std::function<bool(EntityPtr &)> &getter, bool use_transaction) {
		assert(database);
		EntityPtr entity;
		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction)
			transaction.emplace(*database);

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO entities VALUES (?, ?, ?, ?, ?, ?, ?)"};

		while (getter(entity)) {
			if (!entity->shouldPersist() || entity->isPlayer())
				continue;
			statement.bind(1, std::make_signed_t<GlobalID>(entity->getGID()));
			statement.bind(2, entity->realmID);
			statement.bind(3, entity->position.row);
			statement.bind(4, entity->position.column);
			statement.bind(5, entity->type.str());
			statement.bind(6, int(entity->direction.load()));
			Buffer buffer;
			entity->encode(buffer);
			statement.bind(7, buffer.bytes.data(), buffer.bytes.size());
			statement.exec();
			statement.reset();
		}

		if (transaction)
			transaction->commit();
	}

	void GameDB::writeEntities(const RealmPtr &realm, bool use_transaction) {
		decltype(realm->entities)::Base copy;
		{
			auto lock = realm->entities.sharedLock();
			copy = realm->entities.getBase();
		}
		auto iter = copy.begin();
		writeEntities([&](EntityPtr &out) {
			if (iter == copy.end())
				return false;
			out = *iter++;
			return true;
		}, use_transaction);
	}

	void GameDB::deleteEntity(const EntityPtr &entity) {
		assert(database);
		auto db_lock = database.uniqueLock();
		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "DELETE FROM entities WHERE globalID = ?"};
		statement.bind(1, std::make_signed_t<GlobalID>(entity->getGID()));
		statement.exec();
		transaction.commit();
	}

	std::string GameDB::readRealmTilesetHash(RealmID realm_id, bool do_lock) {
		assert(database);

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock)
			db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT tilesetHash FROM realms WHERE realmID = ? LIMIT 1"};

		query.bind(1, realm_id);

		if (query.executeStep())
			return query.getColumn(0).getString();

		throw std::out_of_range("Can't find tileset hash for realm " + std::to_string(realm_id));
	}

	bool GameDB::hasTileset(const std::string &hash, bool do_lock) {
		assert(database);

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock)
			db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT NULL FROM tilesets WHERE hash = ? LIMIT 1"};

		query.bind(1, hash);

		return query.executeStep();
	}

	void GameDB::writeTilesetMeta(const Tileset &tileset, bool use_transaction) {
		assert(database);
		auto db_lock = database.uniqueLock();

		std::optional<SQLite::Transaction> transaction;
		if (use_transaction)
			transaction.emplace(*database);

		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO tilesets VALUES (?, ?)"};

		nlohmann::json json;
		tileset.getMeta(json);

		statement.bind(1, tileset.getHash());
		statement.bind(2, json.dump());

		statement.exec();

		if (transaction)
			transaction->commit();
	}

	bool GameDB::readTilesetMeta(const std::string &hash, nlohmann::json &json, bool do_lock) {
		assert(database);

		std::unique_lock<std::recursive_mutex> db_lock;
		if (do_lock)
			db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT json FROM tilesets WHERE hash = ? LIMIT 1"};

		query.bind(1, hash);

		if (query.executeStep()) {
			json = nlohmann::json::parse(query.getColumn(0).getString());
			return true;
		}

		return false;
	}

	void GameDB::bind(SQLite::Statement &statement, const PlayerPtr &player) {
		statement.bind(1, player->username);
		statement.bind(2, player->displayName);
		statement.bind(3, nlohmann::json(*player).dump());
		statement.bind(4);
		statement.bind(5);
	}
}
