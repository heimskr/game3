#include "lib/JSON.h"
#include "realm/ShadowRealm.h"
#include "worldgen/ShadowRealm.h"

namespace Game3 {
	void ShadowRealm::generateChunk(const ChunkPosition &chunk_position) {
		WorldGen::generateShadowRealm(shared_from_this(), seed, worldgenParams, {chunk_position, chunk_position}, false);
		tileProvider.updateChunk(chunk_position);
	}

	void ShadowRealm::absorbJSON(const boost::json::value &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		worldgenParams = boost::json::value_to<WorldGenParams>(json.at("worldgenParams"));
	}

	void ShadowRealm::toJSON(boost::json::value &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		ensureObject(json)["worldgenParams"] = boost::json::value_from(worldgenParams);
	}
}
