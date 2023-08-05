#include "Log.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "item/ChemicalItem.h"
#include "packet/OpenChemicalReactorPacket.h"
#include "realm/Realm.h"
#include "tileentity/ChemicalReactor.h"

#include <cassert>

namespace Game3 {
	ChemicalReactor::ChemicalReactor():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	ChemicalReactor::ChemicalReactor(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	ChemicalReactor::ChemicalReactor(Position position_):
		ChemicalReactor("base:tile/chemical_reactor"_id, position_) {}

	bool ChemicalReactor::mayInsertItem(const ItemStack &stack, Direction, Slot slot) {
		if (slot != Slot(-1) && Slot(INPUT_CAPACITY) <= slot)
			return false;

		if (auto chemical = std::dynamic_pointer_cast<ChemicalItem>(stack.item))
			return stack.data.contains("formula");

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

	void ChemicalReactor::init(Game &game) {
		TileEntity::init(game);
		inventory = std::make_shared<ServerInventory>(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY);
	}

	void ChemicalReactor::tick(Game &game, float delta) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, game, delta};

		accumulatedTime += delta;

		if (accumulatedTime < PERIOD)
			return;

		accumulatedTime = 0.f;
		react();
	}

	void ChemicalReactor::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		auto equation_lock = const_cast<ChemicalReactor &>(*this).equation.sharedLock();
		if (equation)
			json["equation"] = equation->getText();
	}

	bool ChemicalReactor::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		auto &realm = *getRealm();

		if (modifiers.onlyAlt()) {
			realm.queueDestruction(shared_from_this());
			player->give(ItemStack(realm.getGame(), "base:item/geothermal_generator"_id));
			return true;
		}

		if (modifiers.onlyShift())
			player->send(OpenChemicalReactorPacket(getGID()));
		else if (modifiers.shift && modifiers.ctrl)
			EnergeticTileEntity::addObserver(player);
		else
			InventoriedTileEntity::addObserver(player);

		// DEBUG
		setEquation("H2O -> H + H + O");

		std::shared_lock lock{energyContainer->mutex};
		INFO("Energy: " << energyContainer->energy);
		return true;
	}

	void ChemicalReactor::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		setEquation(json.at("equation"));
	}

	void ChemicalReactor::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		auto lock = equation.uniqueLock();
		std::optional<std::string> equation_text;
		if (equation)
			equation_text = equation->getText();
		buffer << equation_text;
	}

	void ChemicalReactor::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		auto equation_text = buffer.take<std::optional<std::string>>();
		if (equation_text) {
			equation = Chemskr::Equation(*equation_text);
		} else {
			auto lock = equation.uniqueLock();
			equation.reset();
		}
	}

	void ChemicalReactor::broadcast() {
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

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();

		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!EnergeticTileEntity::observers.contains(player))
					player->send(packet);
				return false;
			}

			return true;
		});
	}

	Game & ChemicalReactor::getGame() const {
		return TileEntity::getGame();
	}

	bool ChemicalReactor::setEquation(std::string equation_string) {
		try {

			Chemskr::Equation new_equation(std::move(equation_string));

			if (new_equation.isBalanced()) {
				equation  = std::move(new_equation);
				reactants.clear();
				products.clear();
				return true;
			}

			return false;

		} catch (const Chemskr::InvalidEquationError &) {
			return false;
		}
	}

	bool ChemicalReactor::react() {
		assert(inventory);
		assert(energyContainer);

		{
			auto equation_lock = equation.uniqueLock();
			if (!equation || !equation->isBalanced())
				return false;
		}

		fillReactants();
		fillProducts();

		auto shared_inventory_lock = inventory->sharedLock();

		Game &game = getGame();
		auto &item_registry = game.registry<ItemRegistry>();
		std::shared_ptr<Item> chemical_item = item_registry["base:item/chemical"_id];
		std::unique_ptr<Inventory> inventory_copy = inventory->copy();

		{
			auto reactant_lock = reactants.sharedLock();
			std::vector<ItemStack> stacks;
			auto predicate = [range = SlotRange{0, INPUT_CAPACITY - 1}](Slot slot) {
				return range.contains(slot);
			};

			for (const auto &[reactant, count]: reactants) {
				stacks.emplace_back(game, chemical_item, 1, nlohmann::json{{"formula", reactant}});
				const ItemCount in_inventory = inventory_copy->count(stacks.back(), predicate);
				if (in_inventory < count)
					return false;
			}

			for (const ItemStack &stack: stacks) {
				const ItemCount removed = inventory_copy->remove(stack, predicate);
				if (stack.count != removed)
					throw std::runtime_error("Couldn't remove stack from ChemicalReactor (" + std::to_string(stack.count) + " in stack != " + std::to_string(removed) + " removed)");
			}
		}

		{
			auto products_lock = products.sharedLock();
			auto predicate = [range = SlotRange{INPUT_CAPACITY, INPUT_CAPACITY + OUTPUT_CAPACITY - 1}](Slot slot) {
				return range.contains(slot);
			};

			for (const auto &[product, count]: products)
				if (auto leftover = inventory_copy->add(ItemStack(game, chemical_item, count, nlohmann::json{{"formula", product}}), predicate))
					return false;
		}

		shared_inventory_lock.unlock();

		{
			auto equation_lock = equation.uniqueLock();
			auto energy_lock = energyContainer->uniqueLock();
			EnergyAmount to_consume = equation->getAtomCount() * ENERGY_PER_ATOM;
			if (!energyContainer->remove(to_consume))
				return false;
			EnergeticTileEntity::queueBroadcast();
		}

		// Silly.
		auto storage_inventory = std::dynamic_pointer_cast<StorageInventory>(inventory);
		assert(storage_inventory);
		*storage_inventory = std::move(dynamic_cast<StorageInventory &>(*inventory_copy));

		return true;
	}

	void ChemicalReactor::fillReactants() {
		auto shared_lock = reactants.sharedLock();
		if (reactants.empty()) {
			shared_lock.unlock();
			auto equation_lock = equation.sharedLock();
			assert(equation);
			auto unique_lock = reactants.uniqueLock();
			for (const std::string &reactant: equation->getReactants())
				++reactants[reactant];
		}
	}

	void ChemicalReactor::fillProducts() {
		auto shared_lock = reactants.sharedLock();
		if (products.empty()) {
			shared_lock.unlock();
			auto equation_lock = equation.sharedLock();
			assert(equation);
			auto unique_lock = products.uniqueLock();
			for (const std::string &product: equation->getProducts())
				++products[product];
		}
	}
}
