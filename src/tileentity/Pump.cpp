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
		Pump("base:tile/pump_s"_id, position_) {}

	void Pump::setDirection(Direction new_direction) {
		pumpDirection = new_direction;

		switch (pumpDirection) {
			case Direction::Up:    tileID = "base:tile/pump_n"_id; break;
			case Direction::Right: tileID = "base:tile/pump_e"_id; break;
			case Direction::Down:  tileID = "base:tile/pump_s"_id; break;
			case Direction::Left:  tileID = "base:tile/pump_w"_id; break;
			default:
				tileID = "base:tile/missing"_id;
		}

		cachedTile = -1;
	}

	void Pump::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		json["direction"] = pumpDirection;
	}

	bool Pump::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		auto &realm = *getRealm();
		INFO("Modifiers: " << modifiers);

		if (modifiers.onlyAlt()) {
			INFO("Only alt");
			realm.queueDestruction(shared_from_this());
			player->give(ItemStack(realm.getGame(), "base:item/pump"_id));
			return true;
		}

		if (modifiers.onlyCtrl()) {
			INFO("Only ctrl");
			setDirection(rotateClockwise(getDirection()));
			increaseUpdateCounter();
			broadcast();
			return true;
		}

		INFO("Else");
		// TODO: open fluid level module
		return false;
	}

	void Pump::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		setDirection(json.at("direction"));
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
		buffer << getDirection();
	}

	void Pump::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		setDirection(buffer.take<Direction>());
	}
}
