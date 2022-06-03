#include "game/Building.h"

namespace Game3 {
	void Building::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
	}

	void from_json(const nlohmann::json &json, Building &building) {
		from_json(json, static_cast<TileEntity &>(building));
		building.innerRealmID = json.at("innerRealmID");
	}
}