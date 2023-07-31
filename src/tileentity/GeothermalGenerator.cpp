#include "Log.h"
#include "Tileset.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "item/FilledFlask.h"
// #include "packet/OpenEnergyLevelPacket.h"
#include "realm/Realm.h"
#include "recipe/GeothermalRecipe.h"
#include "tileentity/GeothermalGenerator.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	GeothermalGenerator::GeothermalGenerator():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	GeothermalGenerator::GeothermalGenerator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	GeothermalGenerator::GeothermalGenerator(Position position_):
		GeothermalGenerator("base:tile/geothermal_generator"_id, position_) {}

	bool GeothermalGenerator::mayInsertItem(const ItemStack &stack, Direction, Slot slot) {
		if (slot != 0 && slot != Slot(-1))
			return false;

		auto flask = std::dynamic_pointer_cast<FilledFlask>(stack.item);
		if (!flask)
			return false;

		RealmPtr realm = weakRealm.lock();
		if (!realm)
			return false;

		Game &game = realm->getGame();
		auto &geothermal_registry = game.registry<GeothermalRecipeRegistry>();
		auto &fluid_registry = game.registry<FluidRegistry>();
		std::shared_ptr<Fluid> fluid = fluid_registry.at(flask->fluidName);
		if (!fluid)
			return false;

		return geothermal_registry.fluidIDs.contains(fluid->registryID);
	}

	bool GeothermalGenerator::mayExtractItem(const ItemStack &, Direction, Slot slot) {
		return slot == 1;
	}

	bool GeothermalGenerator::canInsertItem(const ItemStack &stack, Direction direction, Slot slot) {
		if (slot != 0 && slot != Slot(-1))
			return false;

		return mayInsertItem(stack, direction, 0) && inventory->canInsert(stack, 0);
	}

	FluidAmount GeothermalGenerator::getMaxLevel(FluidID id) {
		auto shared_lock = supportedFluids.sharedLock();
		if (supportedFluids)
			return supportedFluids->contains(id)? FLUID_CAPACITY : 0;
		shared_lock.unlock();
		// No data race please :)
		auto unique_lock = supportedFluids.uniqueLock();
		supportedFluids.emplace();
		FluidAmount out = 0;
		for (const std::shared_ptr<GeothermalRecipe> &recipe: getGame().registry<GeothermalRecipeRegistry>().items) {
			supportedFluids->emplace(recipe->input.id);
			if (recipe->input.id == id)
				out = FLUID_CAPACITY;
		}
		return out;
	}

	EnergyAmount GeothermalGenerator::getEnergyCapacity() {
		assert(energyContainer);
		auto lock = energyContainer->sharedLock();
		return energyContainer->capacity;
	}

	void GeothermalGenerator::init(Game &game) {
		TileEntity::init(game);
		inventory = std::make_shared<ServerInventory>(shared_from_this(), 2);
	}

	void GeothermalGenerator::tick(Game &game, float delta) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, game, delta};

		accumulatedTime += delta;

		if (accumulatedTime < PERIOD)
			return;

		accumulatedTime = 0.f;

		assert(fluidContainer);
		assert(energyContainer);
		auto &levels = fluidContainer->levels;
		auto fluid_lock = levels.uniqueLock();

		if (levels.empty())
			return;

		assert(levels.contains(game.registry<FluidRegistry>()["base:fluid/lava"_id]->registryID));
		auto &registry = game.registry<GeothermalRecipeRegistry>();

		std::optional<EnergyAmount> leftovers;
		auto energy_lock = energyContainer->uniqueLock();

		for (const std::shared_ptr<GeothermalRecipe> &recipe: registry.items)
			if (recipe->craft(game, fluidContainer, energyContainer, leftovers))
				return;
	}

	void GeothermalGenerator::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool GeothermalGenerator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		auto &realm = *getRealm();

		if (modifiers.onlyAlt()) {
			realm.queueDestruction(shared_from_this());
			player->give(ItemStack(realm.getGame(), "base:item/geothermal_generator"_id));
			return true;
		}

		if (modifiers.onlyCtrl())
			FluidHoldingTileEntity::addObserver(player);
		else if (modifiers.ctrl && modifiers.shift)
			EnergeticTileEntity::addObserver(player);
		else
			InventoriedTileEntity::addObserver(player);

		{
			assert(fluidContainer);
			auto lock = fluidContainer->levels.sharedLock();
			if (fluidContainer->levels.empty())
				WARN("No fluids.");
			else
				for (const auto &[id, amount]: fluidContainer->levels)
					INFO(realm.getGame().getFluid(id)->identifier << " = " << amount);
		}

		std::shared_lock lock{energyContainer->mutex};
		INFO("Energy: " << energyContainer->energy);
		return false;
	}

	void GeothermalGenerator::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void GeothermalGenerator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void GeothermalGenerator::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void GeothermalGenerator::broadcast() {
		assert(getSide() == Side::Server);

		const TileEntityPacket packet(shared_from_this());

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

	Game & GeothermalGenerator::getGame() const {
		return TileEntity::getGame();
	}
}
