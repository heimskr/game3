#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tileentity/Pump.h"
#include "ui/gl/module/MultiModule.h"

namespace Game3 {
	namespace {
		constexpr float PERIOD = 0.25;
	}

	Pump::Pump():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Pump::Pump(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Pump::Pump(Position position_):
		Pump("base:tile/pump_s"_id, position_) {}

	FluidAmount Pump::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Pump::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};

		enqueueTick(std::chrono::milliseconds(int64_t(1000 * PERIOD)));

		const FluidAmount amount = std::min<FluidAmount>(std::numeric_limits<FluidLevel>::max(), extractionRate * PERIOD);

		if (amount == 0)
			return;

		auto fluid = realm->tryFluid(position + tileDirection);
		if (!fluid)
			return;

		auto energy_lock = energyContainer->uniqueLock();

		FluidLevel to_remove = std::min<FluidLevel>(amount, fluid->level);

		if (ENERGY_PER_UNIT > 0.)
			to_remove = std::min<FluidLevel>(energyContainer->energy / ENERGY_PER_UNIT, to_remove);

		if (to_remove == 0)
			return;

		FluidAmount not_added{};
		{
			auto fluid_lock = fluidContainer->levels.uniqueLock();
			not_added = addFluid(FluidStack(fluid->id, to_remove));
		}
		const FluidAmount removed = to_remove - not_added;

		const EnergyAmount consumed_energy = removed * ENERGY_PER_UNIT;
		assert(consumed_energy <= energyContainer->energy);
		energyContainer->energy -= consumed_energy;
		energy_lock.unlock();

		if (removed == 0)
			return;

		assert(removed <= std::numeric_limits<FluidLevel>::max());

		if (!fluid->isInfinite()) {
			fluid->level -= FluidLevel(removed);
			realm->setFluid(position + tileDirection, *fluid);
		}
	}

	void Pump::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		DirectedTileEntity::toJSON(json);
	}

	bool Pump::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/pump"_id));
			return true;
		}

		if (modifiers.onlyCtrl()) {
			rotateClockwise();
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Energy, Substance::Fluid>::ID(), getGID()));
		FluidHoldingTileEntity::addObserver(player, true);
		EnergeticTileEntity::addObserver(player, true);

		return true;
	}

	void Pump::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		DirectedTileEntity::absorbJSON(game, json);
	}

	void Pump::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		DirectedTileEntity::encode(game, buffer);
	}

	void Pump::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		DirectedTileEntity::decode(game, buffer);
	}

	void Pump::broadcast(bool force) {
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
	}

	GamePtr Pump::getGame() const {
		return TileEntity::getGame();
	}
}
