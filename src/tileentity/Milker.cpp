#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/EnergyContainer.h"
#include "game/ServerGame.h"
#include "game/ServerInventory.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/UpdateAgentFieldPacket.h"
#include "realm/Realm.h"
#include "tileentity/Milker.h"
#include "ui/module/RadiusMachineModule.h"
#include "util/ConstexprHash.h"

namespace Game3 {
	namespace {
		constexpr float PERIOD = 1.0;
		constexpr Radius DEFAULT_RADIUS = 1;
	}

	Milker::Milker():
		EnergeticTileEntity(ENERGY_CAPACITY),
		HasRadius(DEFAULT_RADIUS) {}

	Milker::Milker(Identifier tile_id, Position position):
		TileEntity(std::move(tile_id), ID(), position, true),
		EnergeticTileEntity(ENERGY_CAPACITY),
		HasRadius(DEFAULT_RADIUS) {}

	Milker::Milker(Position position_):
		Milker("base:tile/milker_s"_id, position_) {}

	FluidAmount Milker::getMaxLevel(FluidID) {
		return 64 * FluidTile::FULL;
	}

	void Milker::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();

		if (!realm || realm->getSide() != Side::Server) {
			return;
		}

		Ticker ticker{*this, args};
		enqueueTick(std::chrono::milliseconds(int64_t(1000 * PERIOD)));

		Position center = getPosition() + (Position{} + getDirection()) * static_cast<Index>(getRadius());
		Identifier milk_id;

		realm->findEntitySquare(center, radius, [&](const EntityPtr &entity) {
			milk_id = entity->getMilk();
			return !milk_id.empty();
		});

		if (!milk_id) {
			return;
		}

		GamePtr game = getGame();
		FluidPtr milk = game->getFluid(milk_id);
		assert(milk != nullptr);

		FluidLevel to_extract = FluidTile::FULL;

		{
			auto energy_lock = energyContainer->uniqueLock();

			if (ENERGY_PER_UNIT > 0.) {
				to_extract = std::min<FluidLevel>(energyContainer->energy / ENERGY_PER_UNIT, to_extract);
				if (to_extract == 0) {
					return;
				}
			}

			const EnergyAmount consumed_energy = to_extract * ENERGY_PER_UNIT;
			assert(consumed_energy <= energyContainer->energy);
			energyContainer->energy -= consumed_energy;
		}

		{
			auto fluid_lock = fluidContainer->levels.uniqueLock();
			addFluid(FluidStack(milk->registryID, to_extract));
		}
	}

	void Milker::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		FluidHoldingTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		DirectedTileEntity::toJSON(json);
	}

	bool Milker::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/milker"_id));
			return true;
		}

		if (modifiers.onlyCtrl()) {
			rotateClockwise();
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(RadiusMachineModule::ID(), getGID()));
		FluidHoldingTileEntity::addObserver(player, true);
		EnergeticTileEntity::addObserver(player, true);

		return true;
	}

	void Milker::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		FluidHoldingTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		DirectedTileEntity::absorbJSON(game, json);
	}

	void Milker::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		FluidHoldingTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		DirectedTileEntity::encode(game, buffer);
		buffer << radius;
	}

	void Milker::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		FluidHoldingTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		DirectedTileEntity::decode(game, buffer);
		buffer >> radius;
	}

	bool Milker::setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) {
		switch (field_name) {
			AGENT_FIELD(radius, true);
			default:
				return TileEntity::setField(field_name, field_value, updater);
		}
	}

	void Milker::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		auto packet = make<TileEntityPacket>(getSelf());
		auto energetic_lock = EnergeticTileEntity::observers.uniqueLock();

		std::erase_if(EnergeticTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (PlayerPtr player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});

		auto fluid_holding_lock = FluidHoldingTileEntity::observers.uniqueLock();

		std::erase_if(FluidHoldingTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (PlayerPtr player = weak_player.lock()) {
				if (!EnergeticTileEntity::observers.contains(player)) {
					player->send(packet);
				}
				return false;
			}

			return true;
		});
	}

	GamePtr Milker::getGame() const {
		return TileEntity::getGame();
	}
}
