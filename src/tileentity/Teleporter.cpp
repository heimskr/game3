#include <iostream>

#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Teleporter::Teleporter(Identifier tilename, Position position_, RealmID target_realm, Position target_position):
		TileEntity(std::move(tilename), ID(), std::move(position_), false),
		targetRealm(target_realm),
		targetPosition(std::move(target_position)) {}

	void Teleporter::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["targetRealm"] = targetRealm;
		json["targetPosition"] = targetPosition;
	}

	void Teleporter::onOverlap(const std::shared_ptr<Entity> &entity) {
		entity->teleport(targetPosition, getRealm()->getGame().realms.at(targetRealm));
	}

	void Teleporter::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		targetRealm = json.at("targetRealm");
		targetPosition = json.at("targetPosition");
	}

	void Teleporter::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		auto &realm = *getRealm();
		auto &tileset = realm.getTileset();

		if (tileID != tileset.getEmpty()) {
			const auto tilesize = tileset.getTileSize();
			const auto tile_num = tileset[tileID];
			const auto texture  = tileset.getTexture(realm.getGame());
			const auto x = (tile_num % (*texture->width / tilesize)) * tilesize;
			const auto y = (tile_num / (*texture->width / tilesize)) * tilesize;
			sprite_renderer(*texture, {
				.x = static_cast<float>(position.column),
				.y = static_cast<float>(position.row),
				.x_offset = x / 2.f,
				.y_offset = y / 2.f,
				.size_x = static_cast<float>(tilesize),
				.size_y = static_cast<float>(tilesize),
			});
		}
	}
}
