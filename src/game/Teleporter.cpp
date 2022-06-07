#include <nanogui/opengl.h>

#include <iostream>

#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "game/Teleporter.h"
#include "ui/SpriteRenderer.h"

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

	void Teleporter::render(SpriteRenderer &sprite_renderer) const {
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
