#include "game/Realm.h"

namespace Game3 {
	void to_json(nlohmann::json &json, const Realm &realm) {
		json["id"] = realm.id;
		json["tilemap"] = *realm.tilemap;
		json["tileEntities"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[index, tile_entity]: realm.tileEntities)
			json["tileEntities"][std::to_string(index)] = *tile_entity;
	}
}
