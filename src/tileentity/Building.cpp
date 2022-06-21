#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	void Building::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["innerRealmID"] = innerRealmID;
		json["entrance"] = entrance;
	}

	bool Building::onInteractOn(const std::shared_ptr<Player> &player) {
		return onInteractNextTo(player);
	}

	bool Building::onInteractNextTo(const std::shared_ptr<Player> &player) {
		teleport(player);
		return true;
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

	void Building::render(SpriteRenderer &sprite_renderer) {
		auto realm = getRealm();
		if (tileID != tileSets.at(realm->type)->getEmpty()) {
			auto &tilemap = *realm->tilemap2;
			const auto tilesize = tilemap.tileSize;
			const auto x = (tileID % (tilemap.setWidth / tilesize)) * tilesize;
			const auto y = (tileID / (tilemap.setWidth / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize);
		}
	}
}
