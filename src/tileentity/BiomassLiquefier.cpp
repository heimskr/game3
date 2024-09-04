#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "recipe/BiomassLiquefierRecipe.h"
#include "tileentity/BiomassLiquefier.h"
#include "ui/module/GTKMultiModule.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
		constexpr EnergyAmount ENERGY_CAPACITY = 16'000;
		constexpr EnergyAmount ENERGY_PER_ACTION = 1'000;
		constexpr FluidAmount FLUID_CAPACITY = 64 * FluidTile::FULL;
	}

	BiomassLiquefier::BiomassLiquefier():
		TileEntity(),
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	BiomassLiquefier::BiomassLiquefier(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true),
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	BiomassLiquefier::BiomassLiquefier(Position position_):
		BiomassLiquefier("base:tile/biomass_liquefier"_id, position_) {}

	size_t BiomassLiquefier::getMaxFluidTypes() const {
		return 1;
	}

	FluidAmount BiomassLiquefier::getMaxLevel(FluidID) {
		return FLUID_CAPACITY;
	}

	void BiomassLiquefier::init(Game &game) {
		TileEntity::init(game);
		AgentPtr self = shared_from_this();
		HasInventory::setInventory(Inventory::create(self, 1), 0);
		HasFluids::init(safeDynamicCast<HasFluids>(self));
	}

	void BiomassLiquefier::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};
		enqueueTick(PERIOD);

		const EnergyAmount consumed_energy = ENERGY_PER_ACTION;
		auto energy_lock = energyContainer->uniqueLock();
		if (consumed_energy > energyContainer->energy)
			return;

		InventoryPtr inventory = getInventory(0);
		auto inventory_lock = inventory->uniqueLock();

		ItemStackPtr input = (*inventory)[0];
		if (!input)
			return;

		auto &registry = args.game->registry<BiomassLiquefierRecipeRegistry>();
		auto recipe = registry.maybe(input->getID());
		if (!recipe)
			return;

		auto fluids_lock = fluidContainer->levels.uniqueLock();
		recipe->craft(args.game, inventory, fluidContainer);
	}

	void BiomassLiquefier::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool BiomassLiquefier::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/biomass_liquefier"_id));
			return true;
		}

		player->send(OpenModuleForAgentPacket(GTKMultiModule<Substance::Item, Substance::Energy, Substance::Fluid>::ID(), getGID()));
		EnergeticTileEntity::addObserver(player, true);
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return true;
	}

	void BiomassLiquefier::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void BiomassLiquefier::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void BiomassLiquefier::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void BiomassLiquefier::broadcast(bool force) {
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

	GamePtr BiomassLiquefier::getGame() const {
		return TileEntity::getGame();
	}
}
