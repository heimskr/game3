#include "entity/Player.h"
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

	void Tank::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
	}

	bool Tank::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/tank"_id));
			return true;
		}

		FluidHoldingTileEntity::addObserver(player, false);

		assert(fluidContainer);
		auto fluid_lock = fluidContainer->levels.sharedLock();

		if (fluidContainer->levels.empty()) {
			WARN(2, "No fluids.");
		} else {
			GamePtr game = realm->getGame();
			for (const auto &[id, amount]: fluidContainer->levels)
				INFO(2, "{} = {}", game->getFluid(id)->identifier, amount);
		}

		return false;
	}

	void Tank::absorbJSON(const GamePtr &game, const boost::json::value &json) {
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

	GamePtr Tank::getGame() const {
		return TileEntity::getGame();
	}
}
