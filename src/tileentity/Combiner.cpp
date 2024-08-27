#include "Log.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/InventorySpan.h"
#include "game/ServerInventory.h"
#include "item/ChemicalItem.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "realm/Realm.h"
#include "recipe/CombinerRecipe.h"
#include "tileentity/Combiner.h"
#include "ui/module/CombinerModule.h"

#include <cassert>
#include <numeric>

namespace Game3 {
	namespace {
		constexpr float ENERGY_CAPACITY = 100'000;
		constexpr std::chrono::milliseconds PERIOD{100};
		constexpr ItemCount INPUT_CAPACITY  = 5;
		constexpr ItemCount OUTPUT_CAPACITY = 10;
		constexpr EnergyAmount ENERGY_PER_OPERATION = 500;
	}

	Combiner::Combiner():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Combiner::Combiner(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Combiner::Combiner(Position position_):
		Combiner("base:tile/combiner"_id, position_) {}

	bool Combiner::mayInsertItem(const ItemStackPtr &, Direction, Slot slot) {
		if (slot >= Slot(INPUT_CAPACITY))
			return false;

		return true;
	}

	bool Combiner::mayExtractItem(Direction, Slot slot) {
		return Slot(INPUT_CAPACITY) <= slot && slot < Slot(INPUT_CAPACITY + OUTPUT_CAPACITY);
	}

	EnergyAmount Combiner::getEnergyCapacity() {
		assert(energyContainer);
		auto lock = energyContainer->sharedLock();
		return energyContainer->capacity;
	}

	void Combiner::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
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
				sendMessage(source, "ModuleMessage", CombinerModule::ID(), "TargetSet", success, new_target);

		} else if (name == "GetInventory") {

			Buffer buffer;
			HasInventory::encode(buffer, 0);
			data = std::any(std::move(buffer));

		} else {
			TileEntity::handleMessage(source, name, data);
		}
	}

	void Combiner::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY), 0);
	}

	void Combiner::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};
		combine();
		enqueueTick(PERIOD);
	}

	void Combiner::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool Combiner::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
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
			player->give(ItemStack::create(realm->getGame(), "base:item/combiner"_id));
			return true;
		}

		if (modifiers.onlyShift()) {
			EnergeticTileEntity::addObserver(player, false);
		} else {
			player->send(OpenModuleForAgentPacket(CombinerModule::ID(), getGID()));
			InventoriedTileEntity::addObserver(player, true);
		}

		auto lock = energyContainer->sharedLock();
		return true;
	}

	void Combiner::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void Combiner::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		buffer << target;
	}

	void Combiner::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		setTarget(buffer.take<Identifier>());
	}

	void Combiner::broadcast(bool force) {
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

	GamePtr Combiner::getGame() const {
		return TileEntity::getGame();
	}

	bool Combiner::combine() {
		const InventoryPtr inventory = getInventory(0);

		assert(inventory);
		assert(energyContainer);

		{
			auto energy_lock = energyContainer->sharedLock();
			if (energyContainer->energy < ENERGY_PER_OPERATION)
				return false;
		}

		auto inventory_lock = inventory->uniqueLock();

		GamePtr game = getGame();

		if (!recipe)
			return false;

		std::shared_ptr<Inventory> inventory_copy = inventory->copy();
		auto suppressor = inventory_copy->suppress();

		std::optional<ItemStackPtr> leftover;

		auto input_span  = std::make_shared<InventorySpan>(inventory_copy, 0, INPUT_CAPACITY - 1);
		auto output_span = std::make_shared<InventorySpan>(inventory_copy, INPUT_CAPACITY, INPUT_CAPACITY + OUTPUT_CAPACITY - 1);

		if (!recipe->craft(game, input_span, output_span, leftover) || leftover)
			return false;

		{
			const EnergyAmount to_consume = ENERGY_PER_OPERATION;
			auto energy_lock = energyContainer->uniqueLock();
			if (!energyContainer->remove(to_consume))
				return false;
			EnergeticTileEntity::queueBroadcast();
		}

		// Silly.
		auto storage_inventory = std::dynamic_pointer_cast<StorageInventory>(inventory);
		assert(storage_inventory);
		inventory_copy->weakOwner = shared_from_this();
		suppressor.cancel();
		*storage_inventory = std::move(dynamic_cast<StorageInventory &>(*inventory_copy));
		inventory->notifyOwner();

		return true;
	}

	bool Combiner::setTarget(Identifier new_target) {
		GamePtr game = getGame();
		auto &registry = game->registry<CombinerRecipeRegistry>();

		if (std::shared_ptr<CombinerRecipe> maybe = registry.maybe(new_target)) {
			recipe = std::move(maybe);
			target = std::move(new_target);
			return true;
		}

		return false;
	}
}
