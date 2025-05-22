#include "lib/JSON.h"
#include "realm/Overworld.h"
#include "util/FS.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"

namespace Game3 {
	void Overworld::generateChunk(const ChunkPosition &chunk_position) {
		WorldGen::generateOverworld(shared_from_this(), seed, worldgenParams, {chunk_position, chunk_position}, false);
		tileProvider.updateChunk(chunk_position);
	}

	size_t Overworld::getDefaultSeed() {
		try {
			return parseNumber<size_t>(readFile(".seed"));
		} catch (...) {
			// A decent seed as of v0.41.0.
			return 1000;
		}
	}

	void Overworld::absorbJSON(const boost::json::value &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		worldgenParams = boost::json::value_to<WorldGenParams>(json.at("worldgenParams"));
	}

	void Overworld::toJSON(boost::json::value &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		ensureObject(json)["worldgenParams"] = boost::json::value_from(worldgenParams);
	}
}
