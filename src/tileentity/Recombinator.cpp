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
#include "tileentity/Recombinator.h"
#include "ui/gl/module/MicroscopeModule.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{1'000};
		constexpr EnergyAmount ENERGY_CAPACITY = 16'000;
		constexpr double ENERGY_PER_ACTION = ENERGY_CAPACITY / 8;
	}

	Recombinator::Recombinator():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Recombinator::Recombinator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Recombinator::Recombinator(Position position_):
		Recombinator("base:tile/recombinator"_id, position_) {}

	void Recombinator::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 3), 0);
	}

	namespace {
		bool validateItem(const ItemStackPtr &stack) {
			if (!stack)
				return false;
			const Identifier &id = stack->getID();
			return id == "base:item/gene" || id == "base:item/genetic_template";
		}
	}

	void Recombinator::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};
		enqueueTick(PERIOD);

		const EnergyAmount consumed_energy = ENERGY_PER_ACTION;
		auto energy_lock = energyContainer->uniqueLock();
		if (consumed_energy > energyContainer->energy)
			return;

		if (combine())
			energyContainer->energy -= consumed_energy;
	}

	bool Recombinator::combine() {
		const InventoryPtr inventory = getInventory(0);
		if (!inventory)
			return false;

		auto inventory_lock = inventory->uniqueLock();

		ItemStackPtr first  = (*inventory)[0];
		ItemStackPtr second = (*inventory)[1];
		ItemStackPtr output = (*inventory)[2];

		if (!validateItem(first) || !validateItem(second))
			return false;

		struct CombinedGene {
			boost::json::value data;
			bool fromFirst;

			CombinedGene(boost::json::value data = {}, bool from_first = {}):
				data(std::move(data)), fromFirst(from_first) {}
		};

		bool was_empty = !output;

		if (was_empty) {
			output = ItemStack::create(getGame(), "base:item/genetic_template", 1, boost::json::value{{"genes", std::map<std::string, boost::json::value>()}});
		} else if (output->getID() != "base:item/genetic_template") {
			return false;
		}

		std::map<std::string, CombinedGene> combined_genes;

		for (const ItemStackPtr &stack: {first, second}) {
			if (stack->getID() == "base:item/gene") {
				boost::json::value &gene = stack->data.at("gene");
				combined_genes.try_emplace(gene.at("name"), gene, stack == first);
			} else {
				for (const auto &[name, gene]: stack->data.at("genes").items())
					combined_genes.try_emplace(name, gene, stack == first);
			}
		}

		boost::json::value &genes = output->data.at("genes");

		bool any_from_first  = false;
		bool any_from_second = false;

		for (const auto &[name, combined]: combined_genes) {
			const auto &[gene, from_first] = combined;
			genes[name] = gene;
			if (from_first)
				any_from_first = true;
			else
				any_from_second = true;
		}

		if (any_from_first)
			inventory->erase(0);

		if (any_from_second)
			inventory->erase(1);

		if (was_empty)
			inventory->add(output, 2);

		inventory->notifyOwner(output);
		return any_from_first || any_from_second;
	}

	void Recombinator::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool Recombinator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot slot) {
				if (slot == 0)
					stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/recombinator"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MicroscopeModule<2, Substance::Energy>::ID(), getGID()));
		InventoriedTileEntity::addObserver(player, true);
		EnergeticTileEntity::addObserver(player, true);

		return false;
	}

	void Recombinator::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	bool Recombinator::mayInsertItem(const ItemStackPtr &stack, Direction, Slot slot) {
		return slot < 2 && (stack->getID() == "base:item/gene" || stack->getID() == "base:item/genetic_template");
	}

	void Recombinator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void Recombinator::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void Recombinator::broadcast(bool force) {
		TileEntity::broadcast(force);
	}

	GamePtr Recombinator::getGame() const {
		return TileEntity::getGame();
	}
}
