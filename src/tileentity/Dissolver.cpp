#include "util/Log.h"
#include "entity/Player.h"
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
#include "ui/gl/module/MultiModule.h"

#include <cassert>
#include <numeric>

namespace Game3 {
	namespace {
		constexpr EnergyAmount ENERGY_CAPACITY = 128'000;
		constexpr EnergyAmount ENERGY_PER_ATOM = 64;
		constexpr std::chrono::milliseconds PERIOD{100};
		constexpr ItemCount INPUT_CAPACITY  = 5;
		constexpr ItemCount OUTPUT_CAPACITY = 10;
	}

	Dissolver::Dissolver():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Dissolver::Dissolver(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Dissolver::Dissolver(Position position_):
		Dissolver("base:tile/dissolver"_id, position_) {}

	bool Dissolver::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		if (slot >= Slot(INPUT_CAPACITY))
			return false;

		return std::dynamic_pointer_cast<ChemicalItem>(stack->item) == nullptr;
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
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY), 0);
	}

	void Dissolver::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};
		dissolve();
		enqueueTick(PERIOD);
	}

	void Dissolver::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool Dissolver::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (getSide() == Side::Client)
			return false;

		if (modifiers.onlyAlt()) {
			{
				const InventoryPtr inventory = getInventory(0);
				auto lock = inventory->sharedLock();
				inventory->iterate([&](const ItemStackPtr &stack, Slot) {
					player->give(stack);
					return false;
				});
			}
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/dissolver"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Item, Substance::Energy>::ID(), getGID()));
		EnergeticTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return true;
	}

	void Dissolver::absorbJSON(const GamePtr &game, const boost::json::value &json) {
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

		const auto packet = make<TileEntityPacket>(getSelf());
		const auto energy_packet = makeEnergyPacket();

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

	GamePtr Dissolver::getGame() const {
		return TileEntity::getGame();
	}

	bool Dissolver::dissolve() {
		const InventoryPtr inventory = getInventory(0);

		assert(inventory);
		assert(energyContainer);

		{
			auto energy_lock = energyContainer->sharedLock();
			if (energyContainer->energy < ENERGY_PER_ATOM)
				return false;
		}

		auto inventory_lock = inventory->uniqueLock();

		GamePtr game = getGame();
		DissolverRecipeRegistry &dissolver_registry = game->registry<DissolverRecipeRegistry>();

		ItemStackPtr stack_ptr;
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
		auto suppressor = inventory_copy->suppress();

		std::optional<std::vector<ItemStackPtr>> leftovers;

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
		inventory_copy->setOwner(weak_from_this());
		suppressor.cancel();
		*storage_inventory = std::move(dynamic_cast<StorageInventory &>(*inventory_copy));
		inventory->notifyOwner({});

		return true;
	}
}
