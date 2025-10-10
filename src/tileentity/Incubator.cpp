#include "entity/EntityFactory.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "item/ContainmentOrb.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tileentity/Incubator.h"
#include "ui/module/MicroscopeModule.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{1'000};
		constexpr EnergyAmount ENERGY_CAPACITY = 64'000;
		constexpr EnergyAmount ENERGY_PER_ACTION = 64'000;
		constexpr FluidAmount FLUID_CAPACITY = 64 * FluidTile::FULL;
		constexpr FluidAmount FLUID_PER_ACTION = 8'000;
	}

	Incubator::Incubator():
		TileEntity(),
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Incubator::Incubator(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true),
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Incubator::Incubator(Position position_):
		Incubator("base:tile/incubator"_id, position_) {}

	size_t Incubator::getMaxFluidTypes() const {
		return 1;
	}

	FluidAmount Incubator::getMaxLevel(FluidID) {
		return FLUID_CAPACITY;
	}

	void Incubator::init(Game &game) {
		TileEntity::init(game);
		AgentPtr self = shared_from_this();
		HasInventory::setInventory(Inventory::create(self, 2), 0);
		HasFluids::init(safeDynamicCast<HasFluids>(self));
	}

	void Incubator::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server) {
			return;
		}

		Ticker ticker{*this, args};
		enqueueTick(PERIOD);

		const EnergyAmount consumed_energy = ENERGY_PER_ACTION;
		auto energy_lock = energyContainer->uniqueLock();
		if (consumed_energy > energyContainer->energy) {
			ERR(3, "Not enough energy ({} < {}).", energyContainer->energy, consumed_energy);
			return;
		}

		InventoryPtr inventory = getInventory(0);
		auto inventory_lock = inventory->uniqueLock();

		ItemStackPtr orb = (*inventory)[0];
		if (!ContainmentOrb::validate(orb) || !ContainmentOrb::isEmpty(orb)) {
			ERR(3, "No empty containment orb.");
			return;
		}

		ItemStackPtr genetic_template = (*inventory)[1];
		if (!genetic_template || genetic_template->getID() != "base:item/genetic_template") {
			ERR(3, "No template.");
			return;
		}

		const auto *object = genetic_template->data.if_object();
		if (!object) {
			ERR(3, "Template doesn't have an object as data.");
			return;
		}

		const auto *genes_value = object->if_contains("genes");
		if (!genes_value) {
			ERR(3, "No genes.");
			return;
		}

		GamePtr game = getGame();

		if (!biomassID) {
			biomassID = game->getFluid("base:fluid/liquid_biomass")->registryID;
		}

		auto fluids_lock = fluidContainer->levels.uniqueLock();
		auto fluid_iter = fluidContainer->levels.find(*biomassID);
		if (fluid_iter == fluidContainer->levels.end() || fluid_iter->second < FLUID_PER_ACTION) {
			ERR(3, "Insufficient liquid biomass ({} < {}).", fluid_iter->second, FLUID_PER_ACTION);
			return;
		}

		LivingEntityPtr entity = makeEntity(game, *genes_value);
		if (!entity) {
			ERR(3, "Couldn't make entity.");
			return;
		}

		ContainmentOrb::saveToJSON(entity, orb->data, false);
		fluid_iter->second -= FLUID_PER_ACTION;
		inventory->notifyOwner({});
	}

	void Incubator::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
	}

	bool Incubator::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/incubator"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MicroscopeModule<1, Substance::Energy, Substance::Fluid>::ID(), getGID()));
		EnergeticTileEntity::addObserver(player, true);
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return true;
	}

	void Incubator::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
	}

	void Incubator::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
	}

	void Incubator::decode(Game &game, BasicBuffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
	}

	void Incubator::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const auto packet = make<TileEntityPacket>(getSelf());

		auto energetic_lock = EnergeticTileEntity::observers.uniqueLock();

		std::erase_if(EnergeticTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});

		auto fluid_holding_lock = FluidHoldingTileEntity::observers.uniqueLock();

		std::erase_if(FluidHoldingTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!EnergeticTileEntity::observers.contains(player))
					player->send(packet);
				return false;
			}

			return true;
		});

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();

		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!EnergeticTileEntity::observers.contains(player) && !FluidHoldingTileEntity::observers.contains(player))
					player->send(packet);
				return false;
			}

			return true;
		});
	}

	GamePtr Incubator::getGame() const {
		return TileEntity::getGame();
	}

	LivingEntityPtr Incubator::makeEntity(const GamePtr &game, const boost::json::value &genes) {
		const auto *object = genes.if_object();
		if (!object) {
			ERR(3, "No object.");
			return nullptr;
		}

		auto species_value = object->if_contains("species");
		if (!species_value) {
			ERR(3, "No species.");
			return nullptr;
		}

		Identifier species = boost::json::value_to<Identifier>(species_value->at("value"));
		if (species.empty()) {
			ERR(3, "Empty species.");
			return nullptr;
		}

		auto factory = game->registry<EntityFactoryRegistry>().maybe(species);
		if (!factory) {
			ERR(3, "No factory.");
			return nullptr;
		}

		EntityPtr entity = (*factory)(game);
		if (!entity) {
			ERR(3, "No entity.");
			return nullptr;
		}

		auto living = std::dynamic_pointer_cast<LivingEntity>(entity);
		if (!living) {
			ERR(3, "No living entity.");
			return nullptr;
		}

		if (!living->canAbsorbGenes(genes)) {
			ERR(3, "Can't absorb genes.");
			return nullptr;
		}

		living->absorbGenes(genes);
		return living;
	}
}
