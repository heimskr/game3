#include "game/HasVillages.h"
#include "realm/Realm.h"

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

	void HasVillages::addVillage(ServerGame &game, ChunkPosition chunk_position, const Place &place, const VillageOptions &options) {
		auto lock = villageMap.uniqueLock();
		const auto new_id = getNewVillageID();
		villageMap[new_id] = Village(game, new_id, place.realm->getID(), chunk_position, place.position, options);
		INFO("Added new village " << new_id << " in realm " << place.realm->getID() << " at chunk position " << chunk_position << ", position " << place.position << " with options " << nlohmann::json(options).dump());
	}

	void HasVillages::saveVillages(SQLite::Database &database, bool use_transaction) {
		std::optional<SQLite::Transaction> transaction;
		if (use_transaction)
			transaction.emplace(database);

		SQLite::Statement statement{database, "INSERT OR REPLACE INTO villages VALUES (?, ?, ?, ?, ?, ?, ?)"};

		auto lock = villageMap.sharedLock();
		for (const auto &[id, village]: villageMap) {
			statement.bind(1, int64_t(village.getID()));
			statement.bind(2, village.getRealmID());
			statement.bind(3, std::string(village.getChunkPosition()));
			statement.bind(4, std::string(village.getPosition()));
			statement.bind(5, nlohmann::json(village.options).dump());
			statement.bind(6, nlohmann::json(village.getRichness()).dump());
			statement.bind(7, nlohmann::json(village.getResources()).dump());
			statement.exec();
			statement.reset();
		}
		lock.unlock();

		if (transaction)
			transaction->commit();
	}

	void HasVillages::loadVillages(SQLite::Database &database) {
		SQLite::Statement query{database, "SELECT * FROM villages"};

		auto lock = villageMap.uniqueLock();
		villageMap.clear();
		lastVillageID = 0;

		while (query.executeStep()) {
			size_t id(query.getColumn(0).getInt64());
			RealmID realm_id(query.getColumn(1).getInt());
			ChunkPosition chunk_position(query.getColumn(2).getString());
			Position position(query.getColumn(3).getString());
			VillageOptions options(nlohmann::json::parse(query.getColumn(4).getString()));
			Richness richness(nlohmann::json::parse(query.getColumn(5).getString()));
			Resources resources(nlohmann::json::parse(query.getColumn(6).getString()));
			villageMap[id] = Village(id, realm_id, chunk_position, position, options, std::move(richness), std::move(resources));
			lastVillageID = std::max(lastVillageID.load(), id);
			INFO("Loading village " << id << " in realm " << realm_id << " at chunk position " << chunk_position << ", position " << position << " with options " << nlohmann::json(options).dump());
		}
	}
}
