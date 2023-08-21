#include <iostream>

#include "Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Teleporter::Teleporter(Identifier tilename, Position position_, RealmID target_realm, Position target_position):
		TileEntity(std::move(tilename), ID(), position_, false),
		targetRealm(target_realm),
		targetPosition(target_position) {}

	void Teleporter::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["targetRealm"] = targetRealm;
		json["targetPosition"] = targetPosition;
	}

	void Teleporter::onOverlap(const std::shared_ptr<Entity> &entity) {
		if (getSide() != Side::Server)
			return;

		getRealm()->queue([entity, position = targetPosition, target = getRealm()->getGame().getRealm(targetRealm)] {
			entity->teleport(position, target, MovementContext{.isTeleport = true});
		});
	}

	void Teleporter::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		targetRealm = json.at("targetRealm");
		targetPosition = json.at("targetPosition");
	}

	void Teleporter::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		auto realm = getRealm();
		auto &tileset = realm->getTileset();

		if (tileID != tileset.getEmpty()) {
			const auto tilesize = tileset.getTileSize();
			const auto tile_num = tileset[tileID];
			const auto texture  = tileset.getTexture(realm->getGame());
			const auto x = (tile_num % (texture->width / tilesize)) * tilesize;
			const auto y = (tile_num / (texture->width / tilesize)) * tilesize;
			sprite_renderer(*texture, {
				.x = static_cast<float>(position.column),
				.y = static_cast<float>(position.row),
				.xOffset = static_cast<float>(x) / 2.f,
				.yOffset = static_cast<float>(y) / 2.f,
				.sizeX = static_cast<float>(tilesize),
				.sizeY = static_cast<float>(tilesize),
			});
		}
	}

	void Teleporter::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		buffer << targetRealm;
		buffer << targetPosition;
	}

	void Teleporter::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		buffer >> targetRealm;
		buffer >> targetPosition;
	}
}
