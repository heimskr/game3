#include "Log.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "item/FilledFlask.h"
#include "packet/InteractPacket.h"
#include "realm/Realm.h"
#include "recipe/GeothermalRecipe.h"
#include "tileentity/GeothermalGenerator.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
	}

	GeothermalGenerator::GeothermalGenerator():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	GeothermalGenerator::GeothermalGenerator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	GeothermalGenerator::GeothermalGenerator(Position position_):
		GeothermalGenerator("base:tile/geothermal_generator"_id, position_) {}

	bool GeothermalGenerator::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		if (slot != 0 && slot != Slot(-1))
			return false;

		auto flask = std::dynamic_pointer_cast<FilledFlask>(stack->item);
		if (!flask)
			return false;

		RealmPtr realm = weakRealm.lock();
		if (!realm)
			return false;

		GamePtr game = realm->getGame();
		auto &fluid_registry = game->registry<FluidRegistry>();
		std::shared_ptr<Fluid> fluid = fluid_registry.at(flask->fluidName);
		if (!fluid)
			return false;

		auto &geothermal_registry = game->registry<GeothermalRecipeRegistry>();
		return geothermal_registry.fluidIDs.contains(fluid->registryID);
	}

	bool GeothermalGenerator::mayExtractItem(Direction, Slot slot) {
		return slot == 1;
	}

	bool GeothermalGenerator::canInsertItem(const ItemStackPtr &stack, Direction direction, Slot slot) {
		if (slot != 0 && slot != Slot(-1))
			return false;

		return mayInsertItem(stack, direction, 0) && getInventory(0)->canInsert(stack, 0);
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
		GamePtr game = getGame();
		for (const std::shared_ptr<GeothermalRecipe> &recipe: game->registry<GeothermalRecipeRegistry>().items) {
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
		HasInventory::setInventory(Inventory::create(shared_from_this(), 2), 0);
	}

	void GeothermalGenerator::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};

		enqueueTick(PERIOD);

		assert(fluidContainer);
		assert(energyContainer);

		slurpFlasks();

		auto &levels = fluidContainer->levels;
		auto fluid_lock = levels.uniqueLock();

		if (levels.empty())
			return;

		auto &registry = args.game->registry<GeothermalRecipeRegistry>();

		std::optional<EnergyAmount> leftovers;
		auto energy_lock = energyContainer->uniqueLock();

		for (const std::shared_ptr<GeothermalRecipe> &recipe: registry.items)
			if (recipe->craft(args.game, fluidContainer, energyContainer, leftovers))
				return;
	}

	void GeothermalGenerator::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool GeothermalGenerator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/geothermal_generator"_id));
			return true;
		}

		if (modifiers.onlyCtrl())
			FluidHoldingTileEntity::addObserver(player, false);
		else if (modifiers.onlyShift())
			EnergeticTileEntity::addObserver(player, false);
		else
			InventoriedTileEntity::addObserver(player, false);

		{
			assert(fluidContainer);
			auto lock = fluidContainer->levels.sharedLock();
			if (fluidContainer->levels.empty()) {
				WARN_("No fluids.");
			} else {
				GamePtr game = realm->getGame();
				for (const auto &[id, amount]: fluidContainer->levels)
					INFO("{} = {}", game->getFluid(id)->identifier, amount);
			}
		}

		auto lock = energyContainer->sharedLock();
		INFO("Energy: {}", energyContainer->energy);
		return true;
	}

	void GeothermalGenerator::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	bool GeothermalGenerator::populateMenu(const PlayerPtr &player, bool, const std::string &id, Glib::RefPtr<Gio::Menu> menu, Glib::RefPtr<Gio::SimpleActionGroup> group) {
		auto submenu = Gio::Menu::create();

		Glib::ustring start = "agent" + id;

		Glib::ustring fluid_action = start + ".fluids";
		submenu->append("Fluids", "agent_menu." + fluid_action);
		group->add_action(fluid_action, [this, player] {
			player->send(InteractPacket(false, Hand::None, Modifiers{false, true, false, false}, getGID(), player->direction));
		});

		Glib::ustring energy_action = start + ".energy";
		submenu->append("Energy", "agent_menu." + energy_action);
		group->add_action(energy_action, [this, player] {
			player->send(InteractPacket(false, Hand::None, Modifiers{true, true, false, false}, getGID(), player->direction));
		});

		Glib::ustring inventory_action = start + ".inventory";
		submenu->append("Inventory", "agent_menu." + inventory_action);
		group->add_action(inventory_action, [this, player] {
			player->send(InteractPacket(false, Hand::None, Modifiers{}, getGID(), player->direction));
		});

		menu->append_submenu(getName(), submenu);
		return true;
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

	void GeothermalGenerator::broadcast(bool force) {
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

	GamePtr GeothermalGenerator::getGame() const {
		return TileEntity::getGame();
	}

	void GeothermalGenerator::slurpFlasks() {
		const InventoryPtr inventory = getInventory(0);
		assert(inventory);
		auto inventory_lock = inventory->uniqueLock();

		ItemStackPtr stack = (*inventory)[0];
		if (!stack)
			return;

		auto flask = std::dynamic_pointer_cast<FilledFlask>(stack->item);
		if (!flask)
			return;

		GamePtr game = getGame();
		auto &geothermal_registry = game->registry<GeothermalRecipeRegistry>();
		auto &fluid_registry = game->registry<FluidRegistry>();
		std::shared_ptr<Fluid> fluid = fluid_registry.at(flask->fluidName);
		if (!fluid || !geothermal_registry.fluidIDs.contains(fluid->registryID))
			return;

		const FluidStack fluid_stack = flask->getFluidStack(fluid_registry);
		auto fluid_lock = fluidContainer->levels.uniqueLock();
		const FluidAmount insertable = HasFluids::fluidInsertable(fluid_stack.id);
		const size_t flasks_to_insert = std::min(insertable / fluid_stack.amount, stack->count);

		if (flasks_to_insert == 0)
			return;

		stack->count -= flasks_to_insert;
		if (stack->count == 0)
			inventory->erase(0);

		fluidContainer->levels[fluid_stack.id] += flasks_to_insert * fluid_stack.amount;

		inventory_lock.unlock();
		inventoryUpdated();
		fluidsUpdated();
	}
}
