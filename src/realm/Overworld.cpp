#include <iostream>

#include "realm/Overworld.h"
#include "worldgen/Overworld.h"

namespace Game3 {
	void Overworld::generateChunk(const ChunkPosition &chunk_position) {
		WorldGen::generateOverworld(shared_from_this(), seed, worldgenParams, {chunk_position, chunk_position}, false);
		tileProvider.updateChunk(chunk_position);
	}

	void Overworld::absorbJSON(const nlohmann::json &json) {
		Realm::absorbJSON(json);
		worldgenParams = json.at("worldgenParams");
	}

	void Overworld::toJSON(nlohmann::json &json) const {
		Realm::toJSON(json);
		json["worldgenParams"] = worldgenParams;
	}
}
