#include "entity/Player.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "tileentity/Town.h"

namespace Game3 {
	Town::Town(TileID id_, const Position &position_, RealmID inner_realm_id, Index entrance_, Index width_, Index height_):
	Building(id_, position_, inner_realm_id, entrance_), width(width_), height(height_) {
		tileEntityID = TileEntity::TOWN;
	}

	void Town::toJSON(nlohmann::json &json) const {
		Building::toJSON(json);
		json["width"] = width;
		json["height"] = height;
	}

	void Town::absorbJSON(const nlohmann::json &json) {
		Building::absorbJSON(json);
		width = json.at("width");
		height = json.at("height");
	}
}
