#include "game/TileEntity.h"

namespace Game3 {
	void TileEntity::toJSON(nlohmann::json &json) const {
		json["id"] = id;
		json["pos"] = {row, column};
		json["solid"] = solid;
	}

	void to_json(nlohmann::json &json, const TileEntity &tile_entity) {
		tile_entity.toJSON(json);
	}

	void from_json(const nlohmann::json &json, TileEntity &tile_entity) {
		tile_entity.id = json.at("id");
		tile_entity.row = json.at("pos")[0];
		tile_entity.column = json.at("pos")[1];
		tile_entity.solid = json.at("solid");
	}
}
