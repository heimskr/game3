#include "entity/Player.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "tileentity/Building.h"

namespace Game3 {
	void Building::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
		json["entrance"] = entrance;
	}

	void Building::onInteractNextTo(const std::shared_ptr<Player> &player) {
		player->teleport(entrance, getRealm()->getGame().realms.at(innerRealmID));
	}

	void Building::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		innerRealmID = json.at("innerRealmID");
		entrance = json.at("entrance");
	}
}
