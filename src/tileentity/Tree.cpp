#include <iostream>

#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	// void Tree::toJSON(nlohmann::json &json) const {
	// 	TileEntity::toJSON(json);
	// }

	// void Tree::absorbJSON(const nlohmann::json &json) {
	// 	TileEntity::absorbJSON(json);
	// }

	void Tree::render(SpriteRenderer &sprite_renderer) {
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
