#include <nanogui/opengl.h>

#include <iostream>

#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "game/Sign.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	void Sign::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["text"] = text;
		json["name"] = name;
	}

	void Sign::onInteractNextTo(const std::shared_ptr<Player> &) {
		getRealm()->getGame().setText(text, name, true);
	}

	void Sign::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		text = json.at("text");
		name = json.at("name");
	}

	// void Sign::render(SpriteRenderer &sprite_renderer) const {
	// 	auto realm = getRealm();
	// 	if (tileID != tileSets.at(realm->type)->getEmpty()) {
	// 		auto &tilemap = *realm->tilemap2;
	// 		const auto tilesize = tilemap.tileSize;
	// 		const auto x = (tileID % (tilemap.setWidth / tilesize)) * tilesize;
	// 		const auto y = (tileID / (tilemap.setWidth / tilesize)) * tilesize;
	// 		sprite_renderer.drawOnMap(tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize);
	// 	}
	// }
}
