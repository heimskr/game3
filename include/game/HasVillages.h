#pragma once

#include "game/Village.h"
#include "threading/Lockable.h"

#include <atomic>
#include <cstddef>
#include <map>

namespace SQLite {
	class Database;
}

namespace Game3 {
	struct Place;

	class HasVillages {
		public:
			HasVillages() = default;

			size_t getNewVillageID();
			Village & getVillage(size_t id);
			const Village & getVillage(size_t id) const;
			void addVillage(ServerGame &, ChunkPosition, const Place &, const VillageOptions &);
			void saveVillages(SQLite::Database &, bool use_transaction = true);
			void loadVillages(SQLite::Database &);

		private:
			Lockable<std::map<size_t, Village>> villageMap;
			std::atomic_size_t lastVillageID = 0;
	};
}
