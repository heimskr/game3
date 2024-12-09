#include "biology/Gene.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "item/ContainmentOrb.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tileentity/Sequencer.h"
#include "ui/gl/module/MicroscopeModule.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{5'000};
		constexpr EnergyAmount ENERGY_CAPACITY = 16'000;
		constexpr double ENERGY_PER_ACTION = ENERGY_CAPACITY / 2;
		constexpr Slot OUTPUT_SLOTS = 1;
	}

	Sequencer::Sequencer():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Sequencer::Sequencer(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Sequencer::Sequencer(Position position_):
		Sequencer("base:tile/sequencer"_id, position_) {}

	void Sequencer::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 1 + OUTPUT_SLOTS), 0);
	}

	void Sequencer::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};
		enqueueTick(PERIOD);

		const EnergyAmount consumed_energy = ENERGY_PER_ACTION;
		auto energy_lock = energyContainer->uniqueLock();
		if (consumed_energy > energyContainer->energy)
			return;

		const InventoryPtr inventory = getInventory(0);
		auto inventory_lock = inventory->uniqueLock();

		// We need a filled containment orb in the first slot.
		ItemStackPtr orb = (*inventory)[0];
		if (!ContainmentOrb::validate(orb) || ContainmentOrb::isEmpty(orb))
			return;

		// We need at least one free spot for the gene.
		if (inventory->slotsOccupied() >= inventory->getSlotCount())
			return;

		EntityPtr entity = ContainmentOrb::makeEntity(orb);
		if (!entity)
			return;

		LivingEntityPtr living = std::dynamic_pointer_cast<LivingEntity>(entity);
		if (!living)
			return;

		// Randomly select a gene from what's available.
		std::vector<const Gene *> gene_pointers;

		living->iterateGenes([&gene_pointers](Gene &gene) {
			gene_pointers.push_back(&gene);
		});

		if (gene_pointers.empty())
			return;

		const Gene &gene = *choose(gene_pointers, threadContext.rng);
		ItemStackPtr gene_stack = ItemStack::create(getGame(), "base:item/gene");
		gene.toJSON(gene_stack->data["gene"]);

		const bool has_leftovers = inventory->add(gene_stack) != nullptr;
		assert(!has_leftovers);

		const bool removed = energyContainer->remove(ENERGY_PER_ACTION);
		assert(removed);

		inventory->notifyOwner({});
	}

	void Sequencer::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool Sequencer::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/sequencer"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MicroscopeModule<1, Substance::Energy>::ID(), getGID()));
		InventoriedTileEntity::addObserver(player, true);
		EnergeticTileEntity::addObserver(player, true);

		return false;
	}

	void Sequencer::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	bool Sequencer::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		return slot == 0 && stack->getID() == "base:item/contorb";
	}

	void Sequencer::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void Sequencer::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void Sequencer::broadcast(bool force) {
		TileEntity::broadcast(force);
	}

	GamePtr Sequencer::getGame() const {
		return TileEntity::getGame();
	}
}
