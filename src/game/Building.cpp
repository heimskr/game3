#include <iostream>

#include "entity/Player.h"
#include "game/Building.h"

namespace Game3 {
	void Building::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
		json["entrance"] = entrance;
	}

	void Building::onInteractNextTo(const std::shared_ptr<Player> &player) {
		std::cout << "Player [" << player->id() << "] interacted with Building at position " << position << "\n";
	}

	void Building::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		innerRealmID = json.at("innerRealmID");
		entrance = json.at("entrance");
	}
}
