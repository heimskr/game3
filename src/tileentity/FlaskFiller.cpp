#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tileentity/FlaskFiller.h"
#include "ui/module/MultiModule.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::milliseconds PERIOD{250};
		constexpr EnergyAmount ENERGY_CAPACITY = 16'000;
		constexpr double ENERGY_PER_ACTION = 500;
	}

	FlaskFiller::FlaskFiller():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	FlaskFiller::FlaskFiller(Identifier tile_id, Position position):
		TileEntity(std::move(tile_id), ID(), position, true),
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	FlaskFiller::FlaskFiller(Position position):
		FlaskFiller("base:tile/flask_filler", position) {}

	FluidAmount FlaskFiller::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void FlaskFiller::init(Game &game) {
		HasFluids::init(safeDynamicCast<HasFluids>(shared_from_this()));
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), 5), 0);
	}

	void FlaskFiller::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();

		if (!realm || realm->getSide() != Side::Server) {
			return;
		}

		Ticker ticker{*this, args};
		enqueueTick(PERIOD);

		const EnergyAmount consumed_energy = ENERGY_PER_ACTION;
		auto energy_lock = energyContainer->uniqueLock();
		if (consumed_energy > energyContainer->energy) {
			return;
		}

		GamePtr game = getGame();
		ItemPtr flask = game->getItem("base:item/flask");
		InventoryPtr inventory = getInventory(0);
		auto inventory_lock = inventory->uniqueLock();
		auto fluids_lock = fluidContainer->levels.uniqueLock();

		if (inventory->count(*flask) == 0) {
			return;
		}

		for (auto &[id, amount]: fluidContainer->levels) {
			if (amount < FluidTile::FULL) {
				continue;
			}

			FluidPtr fluid = game->getFluid(id);
			ItemStackPtr filled = ItemStack::create(game, fluid->flaskName, 1);
			if (inventory->add(filled)) {
				// TODO: what if there's all slots are filled except one of the slots contains
				// exactly one empty flask? Could replace that with a filled flask.
				continue;
			}

			inventory->remove(ItemStack::create(game, flask, 1));
			energyContainer->energy -= consumed_energy;
			energyContainer->remove(consumed_energy);
			amount -= FluidTile::FULL;
			if (amount == 0) {
				fluidContainer->levels.erase(id);
			}
			break;
		}
	}

	void FlaskFiller::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
	}

	bool FlaskFiller::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			getInventory(0)->iterate([&](const ItemStackPtr &stack, Slot) {
				stack->spawn(getPlace());
				return false;
			});
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/flask_filler"_id));
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Item, Substance::Energy, Substance::Fluid>::ID(), getGID()));
		EnergeticTileEntity::addObserver(player, true);
		FluidHoldingTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return false;
	}

	void FlaskFiller::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
	}

	void FlaskFiller::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
	}

	void FlaskFiller::decode(Game &game, BasicBuffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
	}

	void FlaskFiller::broadcast(bool force) {
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

	GamePtr FlaskFiller::getGame() const {
		return TileEntity::getGame();
	}
}
