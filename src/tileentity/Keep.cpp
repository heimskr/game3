#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Keep.h"

namespace Game3 {
	Keep::Keep(TileID id_, const Position &position_, RealmID inner_realm_id, Index entrance_, const Position &origin_, Index width_, Index height_):
	Building(id_, position_, inner_realm_id, entrance_), origin(origin_), width(width_), height(height_) {
		tileEntityID = TileEntity::KEEP;
	}

	void Keep::toJSON(nlohmann::json &json) const {
		Building::toJSON(json);
		json["width"] = width;
		json["height"] = height;
		json["origin"] = origin;
	}

	void Keep::absorbJSON(const nlohmann::json &json) {
		Building::absorbJSON(json);
		width = json.at("width");
		height = json.at("height");
		origin = json.at("origin");
	}
}
