#include "entity/Player.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "game/Teleporter.h"

namespace Game3 {
	void Teleporter::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["targetRealm"] = targetRealm;
		json["targetPosition"] = targetPosition;
	}

	void Teleporter::onOverlap(const std::shared_ptr<Entity> &entity) {
		if (auto player = std::dynamic_pointer_cast<Player>(entity)) {
			auto realm = getRealm();
			auto *game = realm->game;
			if (game == nullptr)
				throw std::runtime_error("Realm " + std::to_string(realm->id) + " has a null game");
			player->teleport(targetPosition, game->realms.at(targetRealm));
		}
	}

	void Teleporter::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		targetRealm = json.at("targetRealm");
		targetPosition = json.at("targetPosition");
	}
}
