#include <iostream>

#include "Tileset.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerInventory.h"
#include "packet/OpenFluidLevelsPacket.h"
#include "realm/Realm.h"
#include "tileentity/Pump.h"
#include "ui/SpriteRenderer.h"

namespace Game3 {
	Pump::Pump():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Pump::Pump(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Pump::Pump(Position position_):
		Pump("base:tile/pump_s"_id, position_) {}

	void Pump::setDirection(Direction new_direction) {
		pumpDirection = new_direction;

		switch (pumpDirection) {
			case Direction::Up:    tileID = "base:tile/pump_n"_id; break;
			case Direction::Right: tileID = "base:tile/pump_e"_id; break;
			case Direction::Down:  tileID = "base:tile/pump_s"_id; break;
			case Direction::Left:  tileID = "base:tile/pump_w"_id; break;
			default:
				tileID = "base:tile/missing"_id;
		}

		cachedTile = -1;
	}

	FluidAmount Pump::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Pump::tick(Game &game, float delta) {
		auto realm = weakRealm.lock();
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

		auto fluid = realm->tryFluid(position + pumpDirection);
		if (!fluid)
			return;

		const FluidLevel to_remove = std::min<FluidLevel>(amount, fluid->level);
		if (to_remove == 0)
			return;

		FluidAmount not_added{};
		{
			auto fluid_lock = fluidContainer->levels.uniqueLock();
			not_added = addFluid(FluidStack(fluid->id, to_remove));
		}
		const FluidAmount removed = to_remove - not_added;

		if (removed == 0)
			return;

		assert(removed <= std::numeric_limits<FluidLevel>::max());

		if (!fluid->isInfinite()) {
			fluid->level -= FluidLevel(removed);
			realm->setFluid(position + pumpDirection, *fluid);
		}
	}

	void Pump::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		json["direction"] = pumpDirection;
	}

	bool Pump::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers) {
		auto &realm = *getRealm();

		if (modifiers.onlyAlt()) {
			realm.queueDestruction(shared_from_this());
			player->give(ItemStack(realm.getGame(), "base:item/pump"_id));
			return true;
		}

		if (modifiers.onlyCtrl()) {
			setDirection(rotateClockwise(getDirection()));
			increaseUpdateCounter();
			queueBroadcast(true);
			return true;
		}

		if (modifiers.shift && modifiers.ctrl)
			EnergeticTileEntity::addObserver(player);
		else
			FluidHoldingTileEntity::addObserver(player);

		{
			auto lock = energyContainer->sharedLock();
			INFO("Energy: " << energyContainer->energy);
		}

		{
			auto lock = fluidContainer->levels.sharedLock();
			for (const auto &[id, amount]: fluidContainer->levels)
				INFO(realm.getGame().getFluid(id)->identifier << " = " << amount);
		}

		return false;
	}

	void Pump::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		setDirection(json.at("direction"));
	}

	void Pump::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		buffer << getDirection();
	}

	void Pump::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		setDirection(buffer.take<Direction>());
	}

	void Pump::broadcast() {
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
