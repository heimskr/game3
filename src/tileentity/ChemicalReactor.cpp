#include "util/Log.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "item/ChemicalItem.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "realm/Realm.h"
#include "tileentity/ChemicalReactor.h"
#include "types/SlotRange.h"
#include "ui/gl/module/ChemicalReactorModule.h"

#include <cassert>

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
	}

	ChemicalReactor::ChemicalReactor():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	ChemicalReactor::ChemicalReactor(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	ChemicalReactor::ChemicalReactor(Position position_):
		ChemicalReactor("base:tile/chemical_reactor"_id, position_) {}

	bool ChemicalReactor::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		if (slot >= Slot(INPUT_CAPACITY)) {
			return false;
		}

		if (std::dynamic_pointer_cast<ChemicalItem>(stack->item)) {
			if (const auto *object = stack->data.if_object()) {
				return object->contains("formula");
			}
		}

		return false;
	}

	bool ChemicalReactor::mayExtractItem(Direction, Slot slot) {
		return Slot(INPUT_CAPACITY) <= slot && slot < Slot(INPUT_CAPACITY + OUTPUT_CAPACITY);
	}

	EnergyAmount ChemicalReactor::getEnergyCapacity() {
		assert(energyContainer);
		auto lock = energyContainer->sharedLock();
		return energyContainer->capacity;
	}

	void ChemicalReactor::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "SetEquation") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const std::string new_equation = buffer->take<std::string>();
			const bool success = setEquation(new_equation);
			if (source) {
				sendMessage(source, "ModuleMessage", ChemicalReactorModule::ID(), "EquationSet", success);
			}

		} else {
			TileEntity::handleMessage(source, name, data);
		}
	}

	void ChemicalReactor::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY), 0);
	}

	void ChemicalReactor::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server) {
			return;
		}

		Ticker ticker{*this, args};

		enqueueTick(PERIOD);

		InventoryPtr inventory = getInventory(0);
		if (!inventory->hasOwner()) {
			inventory->setOwner(weak_from_this());
		}

		react();
	}

	void ChemicalReactor::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		auto equation_lock = equation.sharedLock();
		if (equation) {
			json.as_object()["equation"] = boost::json::value_from(equation->getText());
		}
	}

	bool ChemicalReactor::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (getSide() == Side::Client) {
			return false;
		}

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
			player->give(ItemStack::create(realm->getGame(), "base:item/chemical_reactor"_id));
			return true;
		}

		if (modifiers.onlyShift()) {
			EnergeticTileEntity::addObserver(player, false);
		} else {
			player->send(make<OpenModuleForAgentPacket>(ChemicalReactorModule::ID(), getGID()));
			InventoriedTileEntity::addObserver(player, true);
			// TODO: formula observation
		}

		auto lock = energyContainer->sharedLock();
		return true;
	}

	void ChemicalReactor::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		setEquation(std::string(json.at("equation").as_string()));
	}

	void ChemicalReactor::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		auto lock = equation.uniqueLock();
		std::optional<std::string> equation_text;
		if (equation) {
			equation_text = equation->getText();
		}
		buffer << equation_text;
	}

	void ChemicalReactor::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		if (std::optional<std::string> equation_text = buffer.take<std::optional<std::string>>()) {
			setEquation(std::move(*equation_text));
		} else {
			auto lock = equation.uniqueLock();
			equation.reset();
		}
	}

	void ChemicalReactor::broadcast(bool force) {
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

	GamePtr ChemicalReactor::getGame() const {
		return TileEntity::getGame();
	}

	std::string ChemicalReactor::getEquation() {
		auto lock = equation.sharedLock();
		if (equation) {
			return equation->getText();
		}
		return {};
	}

	bool ChemicalReactor::setEquation(std::string equation_string) {
		try {

			Chemskr::Equation new_equation(std::move(equation_string));

			if (new_equation.isBalanced()) {
				equation = std::move(new_equation);
				reactants.clear();
				products.clear();
				return true;
			}

			return false;

		} catch (const Chemskr::InvalidEquationError &) {
			return false;
		} catch (const Chemskr::ParserError &) {
			return false;
		}
	}

	bool ChemicalReactor::hasEquation() {
		auto lock = equation.sharedLock();
		return equation.has_value();
	}

	bool ChemicalReactor::react() {
		const InventoryPtr inventory = getInventory(0);

		assert(inventory);
		assert(energyContainer);

		{
			auto energy_lock = energyContainer->sharedLock();
			if (energyContainer->energy < ENERGY_PER_ATOM) {
				return false;
			}
		}

		{
			auto equation_lock = equation.uniqueLock();
			if (!equation || !equation->isBalanced()) {
				return false;
			}
		}

		fillReactants();
		fillProducts();

		auto shared_inventory_lock = inventory->sharedLock();

		GamePtr game = getGame();
		auto &item_registry = *game->itemRegistry;
		std::shared_ptr<Item> chemical_item = item_registry["base:item/chemical"_id];
		std::unique_ptr<Inventory> inventory_copy = inventory->copy();
		inventory_copy->setOwner({});

		{
			auto reactant_lock = reactants.sharedLock();
			std::vector<ItemStackPtr> stacks;
			auto predicate = [range = SlotRange{0, INPUT_CAPACITY - 1}](const ItemStackPtr &, Slot slot) {
				return range.contains(slot);
			};

			auto slot_predicate = [range = SlotRange{0, INPUT_CAPACITY - 1}](Slot slot) {
				return range.contains(slot);
			};

			for (const auto &[reactant, count]: reactants) {
				stacks.push_back(ItemStack::create(game, chemical_item, count, boost::json::value{{"formula", reactant}}));
				const ItemCount in_inventory = inventory_copy->count(stacks.back(), slot_predicate);
				if (in_inventory < count) {
					return false;
				}
			}

			for (const ItemStackPtr &stack: stacks) {
				const ItemCount removed = inventory_copy->remove(stack, predicate);
				if (stack->count != removed) {
					throw std::runtime_error("Couldn't remove stack from ChemicalReactor (" + std::to_string(stack->count) + " in stack != " + std::to_string(removed) + " removed)");
				}
			}
		}

		{
			auto products_lock = products.sharedLock();
			auto predicate = [range = SlotRange{INPUT_CAPACITY, INPUT_CAPACITY + OUTPUT_CAPACITY - 1}](Slot slot) {
				return range.contains(slot);
			};

			for (const auto &[product, count]: products) {
				if (auto leftover = inventory_copy->add(ItemStack::create(game, chemical_item, count, boost::json::value{{"formula", product}}), predicate)) {
					return false;
				}
			}
		}

		shared_inventory_lock.unlock();

		{
			auto equation_lock = equation.uniqueLock();
			auto energy_lock = energyContainer->uniqueLock();
			EnergyAmount to_consume = equation->getAtomCount() * ENERGY_PER_ATOM;
			if (!energyContainer->remove(to_consume)) {
				return false;
			}
			EnergeticTileEntity::queueBroadcast();
		}

		// Silly.
		auto storage_inventory = std::dynamic_pointer_cast<StorageInventory>(inventory);
		assert(storage_inventory);
		inventory_copy->setOwner(weak_from_this());
		*storage_inventory = std::move(dynamic_cast<StorageInventory &>(*inventory_copy));
		inventory->notifyOwner({});

		return true;
	}

	void ChemicalReactor::fillReactants() {
		auto shared_lock = reactants.sharedLock();
		if (reactants.empty()) {
			shared_lock.unlock();
			auto equation_lock = equation.sharedLock();
			assert(equation);
			auto unique_lock = reactants.uniqueLock();
			for (const auto &[reactant, count]: equation->getReactants()) {
				reactants[reactant] += count;
			}
		}
	}

	void ChemicalReactor::fillProducts() {
		auto shared_lock = reactants.sharedLock();
		if (products.empty()) {
			shared_lock.unlock();
			auto equation_lock = equation.sharedLock();
			assert(equation);
			auto unique_lock = products.uniqueLock();
			for (const auto &[product, count]: equation->getProducts()) {
				products[product] += count;
			}
		}
	}
}
