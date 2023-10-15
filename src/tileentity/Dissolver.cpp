#include "Log.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/InventorySpan.h"
#include "game/ServerInventory.h"
#include "item/ChemicalItem.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "realm/Realm.h"
#include "recipe/DissolverRecipe.h"
#include "tileentity/Dissolver.h"
// #include "ui/module/DissolverModule.h"

#include <cassert>
#include <numeric>

namespace Game3 {
	namespace {
		constexpr float ENERGY_CAPACITY = 100'000;
		constexpr float PERIOD = 0.25;
		constexpr ItemCount INPUT_CAPACITY  = 5;
		constexpr ItemCount OUTPUT_CAPACITY = 10;
		constexpr EnergyAmount ENERGY_PER_ATOM = 100;
	}

	Dissolver::Dissolver():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Dissolver::Dissolver(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Dissolver::Dissolver(Position position_):
		Dissolver("base:tile/dissolver"_id, position_) {}

	bool Dissolver::mayInsertItem(const ItemStack &stack, Direction, Slot slot) {
		if (slot != Slot(-1) && slot >= Slot(INPUT_CAPACITY))
			return false;

		return std::dynamic_pointer_cast<ChemicalItem>(stack.item) == nullptr;
	}

	bool Dissolver::mayExtractItem(Direction, Slot slot) {
		return Slot(INPUT_CAPACITY) <= slot && slot < Slot(INPUT_CAPACITY + OUTPUT_CAPACITY);
	}

	EnergyAmount Dissolver::getEnergyCapacity() {
		assert(energyContainer);
		auto lock = energyContainer->sharedLock();
		return energyContainer->capacity;
	}

	void Dissolver::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY));
	}

	void Dissolver::tick(Game &game, float delta) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, game, delta};

		accumulatedTime += delta;

		if (accumulatedTime < PERIOD)
			return;

		accumulatedTime = 0.f;
		dissolve();
	}

	void Dissolver::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool Dissolver::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
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
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack(realm->getGame(), "base:item/dissolver"_id));
			return true;
		}

		if (modifiers.onlyShift()) {
			EnergeticTileEntity::addObserver(player, false);
		} else {
			// player->send(OpenModuleForAgentPacket(DissolverModule::ID(), getGID()));
			InventoriedTileEntity::addObserver(player, false);
		}

		auto lock = energyContainer->sharedLock();
		INFO("Energy: " << energyContainer->energy);
		return true;
	}

	void Dissolver::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void Dissolver::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void Dissolver::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void Dissolver::broadcast(bool force) {
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

	Game & Dissolver::getGame() const {
		return TileEntity::getGame();
	}

	bool Dissolver::dissolve() {
		const InventoryPtr inventory = getInventory();

		assert(inventory);
		assert(energyContainer);

		{
			auto energy_lock = energyContainer->sharedLock();
			if (energyContainer->energy < ENERGY_PER_ATOM)
				return false;
		}

		auto inventory_lock = inventory->uniqueLock();

		Game &game = getGame();
		DissolverRecipeRegistry &dissolver_registry = game.registry<DissolverRecipeRegistry>();

		ItemStack *stack_ptr = nullptr;
		std::shared_ptr<DissolverRecipe> recipe;

		for (size_t i = 0; i < INPUT_CAPACITY; ++i) {
			stack_ptr = (*inventory)[currentSlot = (currentSlot + 1) % INPUT_CAPACITY];
			if (!stack_ptr)
				continue;

			if (auto optional_recipe = dissolver_registry.maybe(stack_ptr->item->identifier)) {
				recipe = std::move(optional_recipe);
				break;
			}
		}

		if (!recipe)
			return false;

		std::shared_ptr<Inventory> inventory_copy = inventory->copy();
		inventory_copy->weakOwner = {};

		std::optional<std::vector<ItemStack>> leftovers;

		auto input_span  = std::make_shared<InventorySpan>(inventory_copy, 0, INPUT_CAPACITY - 1);
		auto output_span = std::make_shared<InventorySpan>(inventory_copy, INPUT_CAPACITY, INPUT_CAPACITY + OUTPUT_CAPACITY - 1);

		size_t atom_count{};

		if (!recipe->craft(game, input_span, output_span, leftovers, &atom_count) || leftovers)
			return false;

		{
			const EnergyAmount to_consume = atom_count * ENERGY_PER_ATOM;
			auto energy_lock = energyContainer->uniqueLock();
			if (!energyContainer->remove(to_consume))
				return false;
			EnergeticTileEntity::queueBroadcast();
		}

		// Silly.
		auto storage_inventory = std::dynamic_pointer_cast<StorageInventory>(inventory);
		assert(storage_inventory);
		inventory_copy->weakOwner = shared_from_this();
		*storage_inventory = std::move(dynamic_cast<StorageInventory &>(*inventory_copy));
		inventory->notifyOwner();

		return true;
	}
}
