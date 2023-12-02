#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "realm/Realm.h"
#include "tileentity/Pump.h"

namespace Game3 {
	Pump::Pump():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Pump::Pump(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Pump::Pump(Position position_):
		Pump("base:tile/pump_s"_id, position_) {}

	FluidAmount Pump::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Pump::tick(Game &game, float delta) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, game, delta};

		accumulatedTime += delta;

		if (accumulatedTime < PERIOD)
			return;

		const FluidAmount amount = std::min<FluidAmount>(std::numeric_limits<FluidLevel>::max(), extractionRate * accumulatedTime);
		accumulatedTime = 0.f;

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

	void Pump::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		DirectedTileEntity::toJSON(json);
	}

	bool Pump::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, ItemStack *) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack(realm->getGame(), "base:item/pump"_id));
			return true;
		}

		if (modifiers.onlyCtrl()) {
			rotateClockwise();
			return true;
		}

		if (modifiers.shift && modifiers.ctrl)
			EnergeticTileEntity::addObserver(player, false);
		else
			FluidHoldingTileEntity::addObserver(player, false);

		{
			auto lock = energyContainer->sharedLock();
			INFO("Energy: " << energyContainer->energy);
		}

		{
			auto lock = fluidContainer->levels.sharedLock();
			for (const auto &[id, amount]: fluidContainer->levels)
				INFO(realm->getGame().getFluid(id)->identifier << " = " << amount);
		}

		return false;
	}

	void Pump::absorbJSON(Game &game, const nlohmann::json &json) {
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

		const TileEntityPacket packet(getSelf());

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

	Game & Pump::getGame() const {
		return TileEntity::getGame();
	}
}
