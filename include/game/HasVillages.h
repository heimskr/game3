#pragma once

#include "game/Village.h"

#include <atomic>
#include <cstddef>
#include <map>

namespace SQLite {
	class Database;
}

namespace Game3 {
	class HasVillages {
		public:
			HasVillages() = default;

			size_t getNewVillageID();
			Village & getVillage(size_t id);
			const Village & getVillage(size_t id) const;
			void saveVillages(SQLite::Database &, bool use_transaction = true);

		private:
			std::map<size_t, Village> villageMap;
			std::atomic_size_t lastVillageID = 0;
	};
}
