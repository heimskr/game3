#include <iostream>

#include "Texture.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "realm/Realm.h"
#include "tileentity/Pump.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	Pump::Pump(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Pump::Pump(Position position_):
		Pump("base:tile/pump"_id, position_) {}

	void Pump::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
	}

	bool Pump::onInteractNextTo(const PlayerPtr &) {
		return true;
	}

	void Pump::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
	}

	void Pump::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		auto realm = getRealm();
		auto &tileset = realm->getTileset();

		if (cachedTile == TileID(-1))
			cachedTile = tileset[tileID];

		const auto tilesize = tileset.getTileSize();
		const auto texture = tileset.getTexture(realm->getGame());
		const auto x = (cachedTile % (*texture->width / tilesize)) * tilesize;
		const auto y = (cachedTile / (*texture->width / tilesize)) * tilesize;
		sprite_renderer(*texture, {
			.x = static_cast<float>(position.column),
			.y = static_cast<float>(position.row),
			.x_offset = x / 2.f,
			.y_offset = y / 2.f,
			.size_x = static_cast<float>(tilesize),
			.size_y = static_cast<float>(tilesize),
		});
	}

	void Pump::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
	}

	void Pump::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
	}
}
