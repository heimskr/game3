#include <iostream>

#include "realm/ShipRealm.h"
#include "worldgen/ShipRealmGen.h"

namespace Game3 {
	void ShipRealm::generateChunk(const ChunkPosition &chunk_position) {
		WorldGen::generateShipRealmChunks(shared_from_this(), seed, worldgenParams, {chunk_position, chunk_position}, false);
		tileProvider.updateChunk(chunk_position);
	}

	void ShipRealm::absorbJSON(const nlohmann::json &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		worldgenParams = json.at("worldgenParams");
	}

	void ShipRealm::toJSON(nlohmann::json &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		json["worldgenParams"] = worldgenParams;
	}
}
