#include <iostream>

#include "entity/Ship.h"
#include "game/Game.h"
#include "lib/JSON.h"
#include "realm/ShipRealm.h"
#include "worldgen/ShipRealmGen.h"

namespace Game3 {
	ShipRealm::ShipRealm(const std::shared_ptr<Game> &game_, RealmID id_, GlobalID ship_id, int seed_):
		Realm(game_, id_, ID(), "base:tileset/monomap", seed_), shipID(ship_id) {}

	void ShipRealm::generateChunk(const ChunkPosition &chunk_position) {
		WorldGen::generateShipRealmChunks(shared_from_this(), seed, worldgenParams, {chunk_position, chunk_position}, false, getGame()->getAgent<Ship>(shipID));
		tileProvider.updateChunk(chunk_position);
	}

	void ShipRealm::absorbJSON(const boost::json::value &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		worldgenParams = boost::json::value_to<WorldGenParams>(json.at("worldgenParams"));
		shipID = boost::json::value_to<GlobalID>(json.at("shipID"));
	}

	void ShipRealm::toJSON(boost::json::value &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		auto &object = ensureObject(json);
		object["shipID"] = shipID;
		object["worldgenParams"] = boost::json::value_from(worldgenParams);
	}
}
