#pragma once

#include "Types.h"
#include "data/ChunkSet.h"
#include "game/Chunk.h"
#include "game/ChunkPosition.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <leveldb/db.h>

namespace Game3 {
	class Game;
	class Realm;

	class GameDB {
		private:
			Game &game;
			std::unique_ptr<leveldb::DB> database;
			std::filesystem::path path;

			static std::string getChunkKey(RealmID, ChunkPosition);
			static void parseChunkKey(const std::string &, RealmID &, ChunkPosition &);

			static std::string getRealmKey(RealmID);
			static RealmID parseRealmKey(const std::string &);

		public:
			GameDB(Game &);

			void open(std::filesystem::path);
			void close();

			/** Throws if the key isn't found. */
			std::string read(const std::string &key);

			/** Returns `otherwise` if the key isn't found. */
			std::string read(const std::string &key, std::string otherwise);

			void writeAllChunks();
			void writeChunk(const std::shared_ptr<Realm> &, ChunkPosition);

			void readAllChunks();
			/** Reads metadata from the database and returns an empty realm based on the metadata. */
			std::shared_ptr<Realm> loadRealm(RealmID);

			std::optional<ChunkSet> getChunk(RealmID, ChunkPosition);

			inline bool isOpen() {
				return database != nullptr;
			}
	};
}
