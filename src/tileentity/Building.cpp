#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Building::Building(Identifier id_, const Position &position_, RealmID inner_realm_id, Index entrance_):
		TileEntity(std::move(id_), ID(), position_, true),
		innerRealmID(inner_realm_id),
		entrance(entrance_) {}

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

	void Building::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
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
		if (!isVisible())
			return;

		auto realm = getRealm();
		auto &tilemap = *realm->tilemap2;
		const auto &tileset = *tilemap.tileset;

		if (tileID != tilemap.tileset->getEmpty()) {
			const auto tilesize = tilemap.tileSize;
			const auto tile_num = tileset[tileID];
			const auto x = (tile_num % (tilemap.setWidth / tilesize)) * tilesize;
			const auto y = (tile_num / (tilemap.setWidth / tilesize)) * tilesize;
			sprite_renderer.drawOnMap(*tilemap.getTexture(realm->getGame()), position.column, position.row, x / 2, y / 2, tilesize, tilesize);
		}
	}
}
