#pragma once

#include "Types.h"
#include "data/ChunkSet.h"
#include "game/Chunk.h"
#include "game/ChunkPosition.h"
#include "threading/Lockable.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

namespace Game3 {
	class Entity;
	class Game;
	class Player;
	class Realm;
	class TileEntity;

	class GameDB {
		private:
			Game &game;
			std::filesystem::path path;

			void bind(SQLite::Statement &, const std::shared_ptr<Player> &);

		public:
			Lockable<std::unique_ptr<SQLite::Database>> database;

			GameDB(Game &);

			void open(std::filesystem::path);
			void close();

			/** Writes metadata, chunk data, entity data and tile entity data for all realms. */
			void writeAllRealms();

			void writeRealm(const std::shared_ptr<Realm> &);

			void writeChunk(const std::shared_ptr<Realm> &, ChunkPosition, bool use_transaction = true);

			void readAllRealms();

			/** Reads metadata from the database and returns an empty realm based on the metadata. */
			std::shared_ptr<Realm> loadRealm(RealmID, bool do_lock);

			void writeRealmMeta(const std::shared_ptr<Realm> &, bool use_transaction = true);

			std::optional<ChunkSet> getChunk(RealmID, ChunkPosition);

			bool readUser(std::string_view username, std::string *display_name_out, nlohmann::json *json_out);
			void writeUser(std::string_view username, const nlohmann::json &);
			bool hasName(std::string_view username, std::string_view display_name);

			void writeTileEntities(const std::function<bool(std::shared_ptr<TileEntity> &)> &, bool use_transaction = true);
			void writeTileEntities(const std::shared_ptr<Realm> &, bool use_transaction = true);
			void deleteTileEntity(const std::shared_ptr<TileEntity> &);

			void writeEntities(const std::function<bool(std::shared_ptr<Entity> &)> &, bool use_transaction = true);
			void writeEntities(const std::shared_ptr<Realm> &, bool use_transaction = true);
			void deleteEntity(const std::shared_ptr<Entity> &);

			std::string readRealmTilesetHash(RealmID, bool do_lock = true);
			void writeRealmTilesetHash(RealmID, const std::string &, bool use_transaction = true);

			template <template <typename...> typename T>
			T<TileID, Identifier> readRealmTileMap(RealmID realm_id, bool do_lock) {
				assert(database);

				std::unique_lock<std::shared_mutex> db_lock;
				if (do_lock)
					db_lock = database.uniqueLock();

				SQLite::Statement query{*database, "SELECT value FROM realmTileMaps WHERE realmID = ? LIMIT 1"};

				query.bind(1, realm_id);

				while (query.executeStep())
					return nlohmann::json::parse(query.getColumn(0).getString());

				throw std::out_of_range("Can't find tile map for realm " + std::to_string(realm_id));
			}

			template <typename T>
			void writeRealmTileMap(RealmID realm_id, const T &tilemap, bool use_transaction = true) {
				assert(database);
				auto db_lock = database.uniqueLock();

				std::optional<SQLite::Transaction> transaction;
				if (use_transaction)
					transaction.emplace(*database);

				SQLite::Statement statement{*database, "INSERT OR REPLACE INTO realmTileMaps VALUES (?, ?)"};

				statement.bind(1, realm_id);
				statement.bind(2, nlohmann::json(tilemap).dump());

				statement.exec();

				if (transaction)
					transaction->commit();
			}

			inline bool isOpen() {
				return database != nullptr;
			}

			template <typename C>
			void writeUsers(const C &container) {
				if (container.empty())
					return;

				assert(database);
				auto db_lock = database.uniqueLock();

				SQLite::Transaction transaction{*database};
				SQLite::Statement statement{*database, "INSERT OR REPLACE INTO USERS VALUES (?, ?, ?)"};

				for (const std::shared_ptr<Player> player: container) {
					bind(statement, player);
					statement.exec();
					statement.reset();
				}

				transaction.commit();
			}
	};
}
