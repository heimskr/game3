#include <iostream>

#include "Tileset.h"
#include "game/ClientGame.h"
#include "packet/OpenFluidLevelsPacket.h"
#include "realm/Realm.h"
#include "tileentity/Tank.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Tank::Tank(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Tank::Tank(Position position_):
		Tank("base:tile/tank"_id, position_) {}

	FluidAmount Tank::getMaxLevel(FluidID) const {
		return 64 * FluidTile::FULL;
	}

	void Tank::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
	}

	bool Tank::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		auto &realm = *getRealm();

		if (modifiers.onlyAlt()) {
			realm.queueDestruction(shared_from_this());
			player->give(ItemStack(realm.getGame(), "base:item/tank"_id));
			return true;
		}

		player->send(OpenFluidLevelsPacket(getGID()));

		auto lock = fluidLevels.sharedLock();
		if (fluidLevels.empty())
			WARN("No fluids.");
		else
			for (const auto &[id, amount]: fluidLevels)
				INFO(realm.getGame().getFluid(id)->identifier << " = " << amount);
		return false;
	}

	void Tank::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
	}

	void Tank::render(SpriteRenderer &sprite_renderer) {
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

	void Tank::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
	}

	void Tank::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
	}
}
