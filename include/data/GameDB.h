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

			/** Writes both metadata and chunk data for all realms. */
			void writeAllRealms();

			void writeRealm(const std::shared_ptr<Realm> &);

			void writeChunk(const std::shared_ptr<Realm> &, ChunkPosition);

			void readAllRealms();

			/** Reads metadata from the database and returns an empty realm based on the metadata. */
			std::shared_ptr<Realm> loadRealm(RealmID, bool do_lock);

			void writeRealmMeta(const std::shared_ptr<Realm> &);

			std::optional<ChunkSet> getChunk(RealmID, ChunkPosition);

			bool readUser(std::string_view username, std::string *display_name_out, nlohmann::json *json_out);
			void writeUser(std::string_view username, const nlohmann::json &);
			bool hasName(std::string_view username, std::string_view display_name);

			void writeTileEntities(const std::function<bool(std::shared_ptr<TileEntity> &)> &);
			void writeTileEntities(const std::shared_ptr<Realm> &);

			void writeEntities(const std::function<bool(std::shared_ptr<Entity> &)> &);
			void writeEntities(const std::shared_ptr<Realm> &);

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
				SQLite::Statement statement{*database, "INSERT INTO USERS VALUES (?, ?, ?)"};

				for (const std::shared_ptr<Player> player: container) {
					bind(statement, player);
					statement.exec();
					statement.reset();
				}

				transaction.commit();
			}
	};
}
