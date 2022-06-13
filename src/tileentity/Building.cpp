#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"

namespace Game3 {
	void Building::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
		json["entrance"] = entrance;
	}

	void Building::onInteractNextTo(const std::shared_ptr<Player> &player) {
		teleport(player);
	}

	void Building::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		innerRealmID = json.at("innerRealmID");
		entrance = json.at("entrance");
	}

	void Building::teleport(const std::shared_ptr<Entity> &entity) {
		entity->teleport(entrance, getInnerRealm());
	}

	std::shared_ptr<Realm> Building::getInnerRealm() const {
		return getRealm()->getGame().realms.at(innerRealmID);
	}
}
