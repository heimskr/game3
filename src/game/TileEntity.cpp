#include "game/TileEntity.h"
#include "game/Building.h"

namespace Game3 {
	std::shared_ptr<TileEntity> TileEntity::fromJSON(const nlohmann::json &json) {
		const int id = json.at("id");
		switch (id) {
			case Building::ID:
				return std::make_shared<Building>(json);
			default:
				throw std::invalid_argument("Unrecognized TileEntity ID: " + std::to_string(id));
		}
	}

	void TileEntity::toJSON(nlohmann::json &json) const {
		json["id"] = getID();
		json["tileID"] = tileID;
		json["pos"] = {row, column};
		json["solid"] = solid;
	}

	void to_json(nlohmann::json &json, const TileEntity &tile_entity) {
		tile_entity.toJSON(json);
	}

	void from_json(const nlohmann::json &json, TileEntity &tile_entity) {
		tile_entity.tileID = json.at("tileID");
		tile_entity.row = json.at("pos")[0];
		tile_entity.column = json.at("pos")[1];
		tile_entity.solid = json.at("solid");
	}
}
