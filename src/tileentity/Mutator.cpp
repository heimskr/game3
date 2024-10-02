#include "biology/Gene.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tileentity/Mutator.h"
#include "ui/gl/module/MutatorModule.h"

namespace Game3 {
	namespace {
		constexpr FluidAmount FLUID_CAPACITY = 16 * FluidTile::FULL;
		constexpr FluidAmount MUTAGEN_PER_MUTATION = 1'000;
	}

	Mutator::Mutator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Mutator::Mutator(Position position_):
		Mutator("base:tile/mutator"_id, position_) {}

	void Mutator::mutate(float strength) {
		// Ensure we have an inventory.
		InventoryPtr inventory = getInventory(0);
		if (!inventory)
			return;

		// Ensure we have a gene.
		ItemStackPtr stack = (*inventory)[0];
		if (!stack || stack->getID() != "base:item/gene")
			return;

		// Ensure the gene actually has genetic data.
		auto data_lock = stack->data.uniqueLock();
		auto data_iter = stack->data.find("gene");
		if (data_iter == stack->data.end())
			return;

		findMutagen();

		// Remove the mutagen cost from the fluid container if possible; return otherwise.
		{
			auto &levels = fluidContainer->levels;
			auto levels_lock = levels.uniqueLock();
			auto levels_iter = levels.find(*mutagenID);
			if (levels_iter == levels.end())
				return;

			FluidAmount &level = levels_iter->second;

			if (level < MUTAGEN_PER_MUTATION)
				return;

			level -= MUTAGEN_PER_MUTATION;

			if (level == 0)
				levels.erase(levels_iter);
		}

		// Mutate the gene.
		std::unique_ptr<Gene> gene;
		try {
			gene = Gene::fromJSON(*data_iter);
		} catch (const std::exception &err) {
			ERROR("Gene decoding failed in Mutator::mutate: {}", err.what());
		}
		gene->mutate(strength);
		gene->toJSON(*data_iter);
		inventory->notifyOwner({});
	}

	std::unique_ptr<Gene> Mutator::getGene() const {
		InventoryPtr inventory = getInventory(0);
		if (!inventory)
			return nullptr;

		auto inventory_lock = inventory->sharedLock();

		ItemStackPtr stack = (*inventory)[0];
		if (!stack || stack->getID() != "base:item/gene")
			return nullptr;

		auto data_lock = stack->data.sharedLock();
		auto iter = stack->data.find("gene");
		if (iter == stack->data.end())
			return nullptr;

		try {
			return Gene::fromJSON(*iter);
		} catch (const std::exception &err) {
			ERROR("Gene decoding failed in Mutator::getGene: {}", err.what());
			return nullptr;
		}
	}

	void Mutator::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "Mutate") {

			mutate(.4f);

		} else {
			TileEntity::handleMessage(source, name, data);
		}
	}

	void Mutator::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 1), 0);
	}

	void Mutator::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool Mutator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/mutator"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MutatorModule::ID(), getGID()));
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return false;
	}

	void Mutator::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	FluidAmount Mutator::getMaxLevel(FluidID fluid_id) {
		findMutagen();
		return fluid_id == *mutagenID? FLUID_CAPACITY : 0;

	}

	bool Mutator::canInsertFluid(FluidStack stack, Direction) {
		findMutagen();
		return stack.id == *mutagenID;
	}

	bool Mutator::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		return slot == 0 && stack->getID() == "base:item/gene";
	}

	void Mutator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void Mutator::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void Mutator::broadcast(bool force) {
		TileEntity::broadcast(force);
	}

	GamePtr Mutator::getGame() const {
		return TileEntity::getGame();
	}

	void Mutator::findMutagen() {
		if (!mutagenID)
			mutagenID = getGame()->registry<FluidRegistry>().at("base:fluid/mutagen")->registryID;
	}
}
