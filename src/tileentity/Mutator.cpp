#include <iostream>

#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tileentity/Mutator.h"
#include "ui/module/MicroscopeModule.h"

namespace Game3 {
	namespace {
		constexpr FluidAmount FLUID_CAPACITY = 16 * FluidTile::FULL;
	}

	Mutator::Mutator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Mutator::Mutator(Position position_):
		Mutator("base:tile/mutator"_id, position_) {}

	void Mutator::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 1), 0);
	}

	void Mutator::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool Mutator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/mutator"_id));
			return true;
		}

		player->send(OpenModuleForAgentPacket(MicroscopeModule::ID(), getGID()));
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return false;
	}

	void Mutator::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	FluidAmount Mutator::getMaxLevel(FluidID fluid_id) {
		findMutagen();
		return fluid_id == *mutagenID? FLUID_CAPACITY : 0;

	}

	bool Mutator::canInsertFluid(FluidStack stack, Direction) {
		findMutagen();
		return stack.id == *mutagenID;
	}

	bool Mutator::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		const Identifier id = stack->getID();

		if (id == "base:item/gene")
			return slot == 0;

		return false;
	}

	void Mutator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void Mutator::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void Mutator::broadcast(bool force) {
		TileEntity::broadcast(force);
	}

	GamePtr Mutator::getGame() const {
		return TileEntity::getGame();
	}

	void Mutator::findMutagen() {
		if (!mutagenID)
			mutagenID = getGame()->registry<FluidRegistry>().at("base:fluid/mutagen")->registryID;
	}
}
