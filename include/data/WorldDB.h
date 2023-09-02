#pragma once

#include "Types.h"
#include "game/ChunkPosition.h"

#include <filesystem>
#include <memory>
#include <leveldb/db.h>


namespace Game3 {
	class Game;
	class Realm;

	class WorldDB {
		private:
			Game &game;
			std::unique_ptr<leveldb::DB> database;
			std::filesystem::path path;

			std::string getKey(const std::shared_ptr<Realm> &, ChunkPosition);

		public:
			WorldDB(Game &);

			void open(std::filesystem::path);
			void close();

			void writeAll();
			void writeChunk(const std::shared_ptr<Realm> &, ChunkPosition, bool unsafe = false);

			inline bool isOpen() {
				return database != nullptr;
			}
	};
}
