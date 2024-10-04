#include "Log.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/InventorySpan.h"
#include "game/ServerInventory.h"
#include "item/ChemicalItem.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "realm/Realm.h"
#include "tileentity/Disruptor.h"
#include "types/SlotRange.h"
#include "ui/gl/module/MultiModule.h"

#include <cassert>

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
		constexpr float ENERGY_CAPACITY = 100'000;
		constexpr Slot INPUT_CAPACITY  = 5;
		constexpr Slot OUTPUT_CAPACITY = 10;
		constexpr EnergyAmount ENERGY_PER_ATOM = 100;
	}

	Lockable<std::map<std::string, std::map<std::string, size_t>>> Disruptor::atomCounts;

	Disruptor::Disruptor():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Disruptor::Disruptor(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Disruptor::Disruptor(Position position_):
		Disruptor("base:tile/disruptor"_id, position_) {}

	bool Disruptor::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		if (slot >= INPUT_CAPACITY)
			return false;

		if (std::dynamic_pointer_cast<ChemicalItem>(stack->item))
			return stack->data.contains("formula");

		return false;
	}

	bool Disruptor::mayExtractItem(Direction, Slot slot) {
		return INPUT_CAPACITY <= slot && slot < INPUT_CAPACITY + OUTPUT_CAPACITY;
	}

	EnergyAmount Disruptor::getEnergyCapacity() {
		assert(energyContainer);
		auto lock = energyContainer->sharedLock();
		return energyContainer->capacity;
	}

	void Disruptor::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY), 0);
	}

	void Disruptor::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};

		enqueueTick(PERIOD);

		InventoryPtr inventory = getInventory(0);
		if (!inventory->hasOwner()) {
			inventory->setOwner(weak_from_this());
		}

		react();
	}

	void Disruptor::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool Disruptor::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
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
			player->give(ItemStack::create(realm->getGame(), "base:item/disruptor"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Item, Substance::Energy>::ID(), getGID()));
		InventoriedTileEntity::addObserver(player, true);
		EnergeticTileEntity::addObserver(player, true);

		return true;
	}

	void Disruptor::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void Disruptor::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void Disruptor::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void Disruptor::broadcast(bool force) {
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

	GamePtr Disruptor::getGame() const {
		return TileEntity::getGame();
	}

	bool Disruptor::react() {
		const InventoryPtr inventory = getInventory(0);

		assert(inventory);
		assert(energyContainer);

		{
			auto energy_lock = energyContainer->sharedLock();
			if (energyContainer->energy < ENERGY_PER_ATOM)
				return false;
		}

		if (inventory->empty())
			return false;

		GamePtr game = getGame();

		auto inventory_lock = inventory->uniqueLock();
		ItemStackPtr stack;

		Slot slot = 0;
		for (; slot < INPUT_CAPACITY; ++slot) {
			stack = (*inventory)[slot];
			if (stack)
				break;
		}

		if (!stack || stack->getID() != "base:item/chemical")
			return false;

		auto formula_iter = stack->data.find("formula");
		if (formula_iter == stack->data.end())
			return false;

		const std::map<std::string, size_t> *atom_counts = nullptr;
		std::unique_lock<DefaultMutex> atom_lock;

		try {
			atom_counts = &getAtomCounts(atom_lock, *formula_iter);
		} catch (const Chemskr::ParserError &) {
			return false;
		}

		if (!atom_counts)
			return false;

		std::shared_ptr<Item> chemical_item = game->registry<ItemRegistry>()["base:item/chemical"_id];
		std::shared_ptr<Inventory> inventory_copy = inventory->copy();
		inventory_copy->setOwner({});

		InventorySpan output_span(inventory_copy, SlotRange(INPUT_CAPACITY, INPUT_CAPACITY + OUTPUT_CAPACITY - 1));
		size_t total_atom_count = 0;

		for (const auto &[atom, count]: *atom_counts) {
			if (output_span.add(ItemStack::create(game, chemical_item, count, nlohmann::json{{"formula", atom}})))
				return false;
			total_atom_count += count;
		}

		{
			auto energy_lock = energyContainer->uniqueLock();
			EnergyAmount to_consume = total_atom_count * ENERGY_PER_ATOM;
			if (!energyContainer->remove(to_consume))
				return false;
			EnergeticTileEntity::queueBroadcast();
		}

		// Silly.
		auto storage_inventory = std::dynamic_pointer_cast<StorageInventory>(inventory);
		assert(storage_inventory);
		inventory_copy->setOwner(weak_from_this());
		*storage_inventory = std::move(dynamic_cast<StorageInventory &>(*inventory_copy));
		inventory->decrease((*inventory)[slot], slot, 1, false);

		return true;
	}

	const std::map<std::string, size_t> & Disruptor::getAtomCounts(std::unique_lock<DefaultMutex> &lock, const std::string &formula) {
		lock = atomCounts.uniqueLock();

		if (auto iter = atomCounts.find(formula); iter != atomCounts.end())
			return iter->second;

		std::map<std::string, size_t> &counts = atomCounts[formula];
		counts = Chemskr::count(formula);
		return counts;
	}
}
