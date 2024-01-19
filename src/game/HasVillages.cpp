#include "game/HasVillages.h"

#include <nlohmann/json.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include <optional>

namespace Game3 {
	size_t HasVillages::getNewVillageID() {
		return ++lastVillageID;
	}

	Village & HasVillages::getVillage(size_t id) {
		return villageMap.at(id);
	}

	const Village & HasVillages::getVillage(size_t id) const {
		return villageMap.at(id);
	}

	void HasVillages::saveVillages(SQLite::Database &database, bool use_transaction) {
		std::optional<SQLite::Transaction> transaction;
		if (use_transaction)
			transaction.emplace(database);

		SQLite::Statement statement{database, "INSERT OR REPLACE INTO villages VALUES (?, ?, ?, ?)"};

		for (const auto &[id, village]: villageMap) {
			statement.bind(1, int64_t(village.id));
			statement.bind(2, nlohmann::json(village.options).dump());
			statement.bind(3, nlohmann::json(village.richness).dump());
			statement.bind(4, nlohmann::json(village.resources).dump());
			statement.exec();
			statement.reset();
		}

		if (transaction)
			transaction->commit();
	}
}
