#include "entity/Player.h"
#include "game/Building.h"
#include "game/Game.h"
#include "game/Realm.h"

namespace Game3 {
	void Building::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
		json["entrance"] = entrance;
	}

	void Building::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto realm = getRealm();
		auto *game = realm->game;
		if (game == nullptr)
			throw std::runtime_error("Realm " + std::to_string(realm->id) + " has a null game");
		player->teleport(entrance, game->realms.at(innerRealmID));
	}

	void Building::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		innerRealmID = json.at("innerRealmID");
		entrance = json.at("entrance");
	}
}
