#include <iostream>

#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Tank.h"

namespace Game3 {
	Tank::Tank(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Tank::Tank(Position position_):
		Tank("base:tile/tank"_id, position_) {}

	FluidAmount Tank::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Tank::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
	}

	bool Tank::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, ItemStack *, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack(realm->getGame(), "base:item/tank"_id));
			return true;
		}

		FluidHoldingTileEntity::addObserver(player, false);

		assert(fluidContainer);
		auto fluid_lock = fluidContainer->levels.sharedLock();

		if (fluidContainer->levels.empty()) {
			WARN_("No fluids.");
		} else {
			Game &game = realm->getGame();
			for (const auto &[id, amount]: fluidContainer->levels)
				INFO("{} = {}", game.getFluid(id)->identifier, amount);
		}

		return false;
	}

	void Tank::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
	}

	void Tank::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
	}

	void Tank::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
	}

	Game & Tank::getGame() const {
		return TileEntity::getGame();
	}
}
