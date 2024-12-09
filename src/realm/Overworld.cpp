#include <iostream>

#include "realm/Overworld.h"
#include "worldgen/Overworld.h"

namespace Game3 {
	void Overworld::generateChunk(const ChunkPosition &chunk_position) {
		WorldGen::generateOverworld(shared_from_this(), seed, worldgenParams, {chunk_position, chunk_position}, false);
		tileProvider.updateChunk(chunk_position);
	}

	void Overworld::absorbJSON(const boost::json::value &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		worldgenParams = json.at("worldgenParams");
	}

	void Overworld::toJSON(boost::json::value &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		json["worldgenParams"] = worldgenParams;
	}
}
