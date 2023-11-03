#include "game/EnergyContainer.h"
#include "game/Game.h"
#include "game/InventorySpan.h"
#include "game/ServerInventory.h"
#include "item/Furniture.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/Autocrafter.h"
#include "ui/module/AutocrafterModule.h"

namespace Game3 {
	namespace {
		constexpr EnergyAmount ENERGY_CAPACITY = 100'000;
		constexpr EnergyAmount ENERGY_PER_ACTION = 200;
		constexpr float PERIOD = 0.1;
		constexpr ItemCount INPUT_CAPACITY  = 15;
		constexpr ItemCount OUTPUT_CAPACITY = 15;
	}

	Autocrafter::Autocrafter():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Autocrafter::Autocrafter(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Autocrafter::Autocrafter(Position position_):
		TileEntity("base:tile/autocrafter", ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	bool Autocrafter::mayInsertItem(const ItemStack &, Direction, Slot slot) {
		return slot == Slot(-1) || slot < Slot(INPUT_CAPACITY);
	}

	bool Autocrafter::mayExtractItem(Direction, Slot slot) {
		return Slot(INPUT_CAPACITY) <= slot && slot < Slot(INPUT_CAPACITY + OUTPUT_CAPACITY);
	}

	EnergyAmount Autocrafter::getEnergyCapacity() {
		assert(energyContainer);
		return energyContainer->copyCapacity();
	}

	void Autocrafter::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY));
	}

	void Autocrafter::tick(Game &game, float delta) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, game, delta};

		accumulatedTime += delta;

		if (accumulatedTime < PERIOD)
			return;

		accumulatedTime = 0.f;
		autocraft();
	}

	bool Autocrafter::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		if (getSide() == Side::Client)
			return false;

		if (modifiers.onlyAlt()) {
			{
				const InventoryPtr inventory = getInventory();
				auto lock = inventory->sharedLock();
				inventory->iterate([&](const ItemStack &stack, Slot) {
					player->give(stack);
					return false;
				});
			}
			if (auto lock = stationStack.uniqueLock(); stationStack)
				player->give(std::move(*stationStack));
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack(realm->getGame(), "base:item/autocrafter"_id));
			return true;
		}

		if (modifiers.onlyShift()) {
			EnergeticTileEntity::addObserver(player, false);
		} else {
			player->send(OpenModuleForAgentPacket(AutocrafterModule::ID(), getGID()));
			InventoriedTileEntity::addObserver(player, true);
		}

		auto lock = energyContainer->sharedLock();
		return true;
	}

	void Autocrafter::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		if (stationStack)
			json["stationStack"] = *stationStack;
	}

	void Autocrafter::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		if (auto iter = json.find("stationStack"); iter != json.end())
			stationStack = ItemStack::fromJSON(game, *iter);
	}

	void Autocrafter::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		buffer << stationStack;
	}

	void Autocrafter::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		setStation(buffer.take<std::optional<ItemStack>>());
	}

	void Autocrafter::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const TileEntityPacket packet(getSelf());
		const SetTileEntityEnergyPacket energy_packet = makeEnergyPacket();

		{
			auto energetic_lock = EnergeticTileEntity::observers.uniqueLock();
			std::erase_if(EnergeticTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
				if (auto player = weak_player.lock()) {
					player->send(energy_packet);
					return false;
				}

				return true;
			});
		}

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();
		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});
	}

	void Autocrafter::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "SetTarget") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			bool success = false;

			Identifier new_target;

			try {
				new_target = buffer->take<Identifier>();
				success = setTarget(new_target);
			} catch (const std::invalid_argument &) {}

			if (source)
				sendMessage(source, "ModuleMessage", AutocrafterModule::ID(), "TargetSet", success, new_target);

		}
	}

	void Autocrafter::autocraft() {
		if (energyContainer->copyEnergy() < ENERGY_PER_ACTION)
			return;

		auto recipes_lock = cachedRecipes.sharedLock();
		if (cachedRecipes.empty())
			return;

		InventoryPtr inventory = getInventory();
		const ItemCount input_capacity = INPUT_CAPACITY;
		auto input_span = std::make_shared<InventorySpan>(inventory, 0, input_capacity - 1);
		auto output_span = std::make_shared<InventorySpan>(inventory, input_capacity, input_capacity + OUTPUT_CAPACITY - 1);
		Game &game = getGame();

		std::optional<std::vector<ItemStack>> leftovers;
		for (const std::shared_ptr<CraftingRecipe> &recipe: cachedRecipes) {
			if (recipe->craft(game, input_span, output_span, leftovers)) {
				auto energy_lock = energyContainer->sharedLock();
				energyContainer->remove(ENERGY_PER_ACTION, true);
				return;
			}
		}
	}

	void Autocrafter::cacheRecipes() {
		auto lock = cachedRecipes.uniqueLock();
		cachedRecipes.clear();
		for (const std::shared_ptr<CraftingRecipe> &recipe: getGame().registry<CraftingRecipeRegistry>())
			if (validateRecipe(*recipe))
				cachedRecipes.push_back(recipe);
	}

	bool Autocrafter::setStation(std::optional<ItemStack> stack) {
		auto stack_lock = stationStack.uniqueLock();
		stationStack.getBase() = std::move(stack);

		bool out = true;

		if (stationStack) {
			if (auto station_item = std::dynamic_pointer_cast<StationFurniture>(stationStack->item)) {
				station = station_item->stationType;
			} else {
				stationStack.reset();
				station = {};
				out = false;
			}
		} else {
			station = {};
		}

		cacheRecipes();
		return out;
	}

	bool Autocrafter::validateRecipe(const CraftingRecipe &recipe) const {
		if (!recipe.stationType.empty() && recipe.stationType != station)
			return false;

		auto target_lock = target.sharedLock();

		for (const ItemStack &stack: recipe.output)
			if (stack.item->identifier == target)
				return true;

		return false;
	}
}
