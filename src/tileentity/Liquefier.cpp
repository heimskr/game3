#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "recipe/LiquefierRecipe.h"
#include "tileentity/Liquefier.h"
#include "ui/module/MultiModule.h"
#include "util/Cast.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
		constexpr EnergyAmount ENERGY_CAPACITY = 16'000;
		constexpr double ENERGY_PER_ACTION = 500;
	}

	Liquefier::Liquefier():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Liquefier::Liquefier(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Liquefier::Liquefier(Position position_):
		Liquefier("base:tile/liquefier"_id, position_) {}

	FluidAmount Liquefier::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Liquefier::init(Game &game) {
		HasFluids::init(safeDynamicCast<HasFluids>(shared_from_this()));
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 5), 0);
	}

	void Liquefier::tick(const TickArgs &args) {
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

		for (const std::shared_ptr<LiquefierRecipe> &recipe: args.game->registry<LiquefierRecipeRegistry>().items) {
			if (recipe->craft(args.game, inventory, fluidContainer)) {
				energyContainer->energy -= consumed_energy;
				return;
			}
		}
	}

	void Liquefier::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool Liquefier::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/liquefier"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Item, Substance::Energy, Substance::Fluid>::ID(), getGID()));
		EnergeticTileEntity::addObserver(player, true);
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return false;
	}

	void Liquefier::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	void Liquefier::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void Liquefier::decode(Game &game, BasicBuffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void Liquefier::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const auto packet = make<TileEntityPacket>(getSelf());

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

	GamePtr Liquefier::getGame() const {
		return TileEntity::getGame();
	}
}
