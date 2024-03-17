#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "recipe/LiquifierRecipe.h"
#include "tileentity/Liquifier.h"
#include "ui/module/MultiModule.h"
#include "util/Cast.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
		constexpr EnergyAmount ENERGY_CAPACITY = 16'000;
		constexpr double ENERGY_PER_ACTION = 200;
	}

	Liquifier::Liquifier():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Liquifier::Liquifier(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Liquifier::Liquifier(Position position_):
		Liquifier("base:tile/liquifier"_id, position_) {}

	FluidAmount Liquifier::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Liquifier::init(Game &game) {
		HasFluids::init(safeDynamicCast<HasFluids>(shared_from_this()));
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 5), 0);
	}

	void Liquifier::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};
		enqueueTick(PERIOD);

		const EnergyAmount consumed_energy = ENERGY_PER_ACTION;
		auto energy_lock = energyContainer->uniqueLock();
		if (consumed_energy > energyContainer->energy)
			return;

		const InventoryPtr inventory = getInventory(0);
		auto inventory_lock = inventory->uniqueLock();
		auto fluids_lock = fluidContainer->levels.uniqueLock();

		for (const std::shared_ptr<LiquifierRecipe> &recipe: args.game->registry<LiquifierRecipeRegistry>().items) {
			if (recipe->craft(args.game, inventory, fluidContainer)) {
				energyContainer->energy -= consumed_energy;
				return;
			}
		}
	}

	void Liquifier::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool Liquifier::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/liquifier"_id));
			return true;
		}

		player->send(OpenModuleForAgentPacket(MultiModule<Substance::Item, Substance::Energy, Substance::Fluid>::ID(), getGID()));
		EnergeticTileEntity::addObserver(player, true);
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return false;
	}

	void Liquifier::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	void Liquifier::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void Liquifier::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void Liquifier::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const TileEntityPacket packet(getSelf());

		auto energetic_lock = EnergeticTileEntity::observers.uniqueLock();

		std::erase_if(EnergeticTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});

		auto fluid_holding_lock = FluidHoldingTileEntity::observers.uniqueLock();

		std::erase_if(FluidHoldingTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!EnergeticTileEntity::observers.contains(player))
					player->send(packet);
				return false;
			}

			return true;
		});

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();

		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!EnergeticTileEntity::observers.contains(player) && !FluidHoldingTileEntity::observers.contains(player))
					player->send(packet);
				return false;
			}

			return true;
		});
	}

	GamePtr Liquifier::getGame() const {
		return TileEntity::getGame();
	}
}
