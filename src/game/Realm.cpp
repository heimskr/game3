#include "game/Realm.h"
#include "util/Util.h"

namespace Game3 {
	void to_json(nlohmann::json &json, const Realm &realm) {
		json["id"] = realm.id;
		json["tilemap"] = *realm.tilemap;
		json["tileEntities"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[index, tile_entity]: realm.tileEntities)
			json["tileEntities"][std::to_string(index)] = *tile_entity;
	}

	void from_json(const nlohmann::json &json, Realm &realm) {
		realm.id = json.at("id");
		realm.tilemap = std::make_shared<Tilemap>(json.at("tilemap"));
		for (const auto &[index, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>()) {
			realm.tileEntities.emplace(parseUlong(index), TileEntity::fromJSON(tile_entity_json));
		}
	}
}
