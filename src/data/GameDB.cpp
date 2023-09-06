#include "data/GameDB.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "util/Endian.h"
#include "util/Util.h"

#include <iomanip>
#include <sstream>

#include <nlohmann/json.hpp>

namespace Game3 {
	GameDB::GameDB(Game &game_):
		game(game_) {}

	void GameDB::open(std::filesystem::path path_) {
		close();
		path = std::move(path_);
		auto db_lock = database.uniqueLock();
		database.getBase() = std::make_unique<SQLite::Database>(path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

		static_assert(LAYER_COUNT * sizeof(TileID) * CHUNK_SIZE * CHUNK_SIZE < 65536);
		static_assert(sizeof(BiomeType) * CHUNK_SIZE * CHUNK_SIZE < 65536);
		static_assert(sizeof(FluidInt) * CHUNK_SIZE * CHUNK_SIZE < 65536);

		database->exec(R"(
			CREATE TABLE IF NOT EXISTS chunks (
				realmID INT,
				x INT,
				y INT,
				terrain VARBINARY(65535),
				biomes  VARBINARY(65535),
				fluids  VARBINARY(65535),
				PRIMARY KEY (realmID, x, y)
			);

			CREATE TABLE IF NOT EXISTS realms (
				realmID INT PRIMARY KEY,
				json MEDIUMTEXT
			);

			CREATE TABLE IF NOT EXISTS users (
				username VARCHAR(32) PRIMARY KEY,
				displayName VARCHAR(64),
				json MEDIUMTEXT
			);
		)");
	}

	void GameDB::close() {
		database.reset();
	}

	void GameDB::writeAllRealms() {
		game.iterateRealms([this](const RealmPtr &realm) {
			writeRealmMeta(realm);
			std::shared_lock lock(realm->tileProvider.chunkMutexes[0]);
			for (const auto &[chunk_position, chunk]: realm->tileProvider.chunkMaps[0])
				writeChunk(realm, chunk_position);
		});
	}

	void GameDB::writeChunk(const RealmPtr &realm, ChunkPosition chunk_position) {
		assert(database);
		TileProvider &provider = realm->tileProvider;

		auto db_lock = database.uniqueLock();

		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO chunks VALUES (?, ?, ?, ?, ?, ?)"};

		statement.bind(1, realm->id);
		statement.bind(2, chunk_position.x);
		statement.bind(3, chunk_position.y);
		statement.bind(4, provider.getRawTerrain(chunk_position));
		statement.bind(5, provider.getRawBiomes(chunk_position));
		statement.bind(6, provider.getRawFluids(chunk_position));

		statement.exec();
		transaction.commit();
	}

	void GameDB::readAllRealms() {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT * FROM chunks"};

		while (query.executeStep()) {
			RealmID realm_id = query.getColumn(0);
			ChunkPosition::IntType x = query.getColumn(1);
			ChunkPosition::IntType y = query.getColumn(2);
			const void *terrain = query.getColumn(3);
			const void *biomes  = query.getColumn(4);
			const void *fluids  = query.getColumn(5);
			ChunkSet chunk_set{
				std::span<const char>(reinterpret_cast<const char *>(terrain), query.getColumn(3).getBytes()),
				std::span<const char>(reinterpret_cast<const char *>(biomes),  query.getColumn(4).getBytes()),
				std::span<const char>(reinterpret_cast<const char *>(fluids),  query.getColumn(5).getBytes())
			};
			INFO("RealmID: " << realm_id);
			RealmPtr realm = game.getRealm(realm_id, [&] { return loadRealm(realm_id, false); });
			realm->tileProvider.absorb(ChunkPosition(x, y), chunk_set);
		}
	}

	RealmPtr GameDB::loadRealm(RealmID realm_id, bool do_lock) {
		assert(database);

		std::unique_lock<std::shared_mutex> db_lock;
		if (do_lock)
			db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT json FROM realms WHERE realmID = ? LIMIT 1"};

		query.bind(1, realm_id);

		while (query.executeStep())
			return Realm::fromJSON(game, nlohmann::json::parse(query.getColumn(0).getString()), false);

		throw std::out_of_range("Couldn't find realm " + std::to_string(realm_id) + " in database");
	}

	void GameDB::writeRealmMeta(const std::shared_ptr<Realm> &realm) {
		assert(database);

		nlohmann::json json;
		realm->toJSON(json, false);

		auto db_lock = database.uniqueLock();

		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO realms VALUES (?, ?)"};

		statement.bind(1, realm->id);
		statement.bind(2, json.dump());

		statement.exec();
		transaction.commit();

	}

	std::optional<ChunkSet> GameDB::getChunk(RealmID realm_id, ChunkPosition chunk_position) {
		assert(database);

		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT terrain, biomes, fluids FROM chunks WHERE realmID = ? AND x = ? AND y = ? LIMIT 1"};

		query.bind(1, realm_id);
		query.bind(2, chunk_position.x);
		query.bind(3, chunk_position.y);

		while (query.executeStep()) {
			const void *terrain = query.getColumn(0);
			const void *biomes  = query.getColumn(1);
			const void *fluids  = query.getColumn(2);
			return ChunkSet{
				std::span<const char>(reinterpret_cast<const char *>(terrain), query.getColumn(0).getBytes()),
				std::span<const char>(reinterpret_cast<const char *>(biomes),  query.getColumn(1).getBytes()),
				std::span<const char>(reinterpret_cast<const char *>(fluids),  query.getColumn(2).getBytes())
			};
		}

		return std::nullopt;
	}

	bool GameDB::readUser(std::string_view username, std::string *display_name_out, nlohmann::json *json_out) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT displayName, json FROM users WHERE username = ? LIMIT 1"};

		query.bind(1, username.data());

		while (query.executeStep()) {
			if (display_name_out != nullptr)
				*display_name_out = std::string(query.getColumn(0));
			if (json_out != nullptr)
				*json_out = nlohmann::json::parse(std::string(query.getColumn(1)));
			return true;
		}

		return false;
	}

	void GameDB::writeUser(std::string_view username, const nlohmann::json &json) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Transaction transaction{*database};
		SQLite::Statement statement{*database, "INSERT OR REPLACE INTO USERS VALUES (?, ?, ?)"};

		statement.bind(1, username.data());
		statement.bind(2, json.at("displayName").get<std::string>());
		statement.bind(3, json.dump());

		statement.exec();
		transaction.commit();
	}

	bool GameDB::hasName(std::string_view username, std::string_view display_name) {
		assert(database);
		auto db_lock = database.uniqueLock();

		SQLite::Statement query{*database, "SELECT NULL FROM users WHERE username = ? OR displayName = ? LIMIT 1"};

		query.bind(1, username.data());
		query.bind(2, display_name.data());

		return query.executeStep();
	}

	void GameDB::bind(SQLite::Statement &statement, const std::shared_ptr<Player> &player) {
		statement.bind(1, player->username);
		statement.bind(2, player->displayName);
		statement.bind(3, nlohmann::json(*player).dump());
	}
}
