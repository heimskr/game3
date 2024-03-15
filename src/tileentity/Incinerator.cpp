#include <iostream>

#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "tileentity/Incinerator.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{100};
	}

	Incinerator::Incinerator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Incinerator::Incinerator(Position position_):
		Incinerator("base:tile/incinerator"_id, position_) {}

	size_t Incinerator::getMaxFluidTypes() const {
		return 16;
	}

	FluidAmount Incinerator::getMaxLevel(FluidID) {
		return 1'000 * FluidTile::FULL;
	}

	void Incinerator::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 64), 0);
	}

	void Incinerator::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};

		enqueueTick(PERIOD);

		{
			auto lock = fluidContainer->levels.uniqueLock();
			fluidContainer->levels.clear();
		}

		{
			InventoryPtr inventory = getInventory(0);
			auto lock = inventory->uniqueLock();
			inventory->clear();
		}
	}

	void Incinerator::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool Incinerator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/incinerator"_id));
			return true;
		}

		return false;
	}

	void Incinerator::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	void Incinerator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		// No need to encode inventory/fluid contents—they're supposed to be discarded immediately anyway.
	}

	void Incinerator::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		// Same as above.
	}

	void Incinerator::broadcast(bool force) {
		TileEntity::broadcast(force);
	}

	GamePtr Incinerator::getGame() const {
		return TileEntity::getGame();
	}
}
