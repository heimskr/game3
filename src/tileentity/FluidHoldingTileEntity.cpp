#include "game/ClientGame.h"
#include "packet/OpenFluidLevelsPacket.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	FluidHoldingTileEntity::FluidHoldingTileEntity(FluidContainer::Map map):
		HasFluids(std::make_shared<FluidContainer>(std::move(map))) {}

	bool FluidHoldingTileEntity::canInsertFluid(FluidStack stack, Direction) {
		return canInsertFluid(stack);
	}

	FluidAmount FluidHoldingTileEntity::addFluid(FluidStack stack, Direction) {
		return addFluid(stack);
	}

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction, const std::function<bool(FluidID)> &predicate, bool remove, const std::function<FluidAmount(FluidID)> &max_amount) {
		auto &levels = fluidContainer->levels;
		{
			auto lock = levels.sharedLock();
			if (levels.empty())
				return std::nullopt;
		}

		auto lock = levels.uniqueLock();
		auto iter = levels.begin();

		// Advance through possible fluids until we find an acceptable one.
		if (predicate)
			while (iter != levels.end() && !predicate(iter->first))
				++iter;

		if (iter == levels.end())
			return std::nullopt;

		const FluidID id = iter->first;
		const FluidAmount to_remove = std::min(max_amount? max_amount(id) : std::numeric_limits<FluidAmount>::max(), iter->second);

		if (remove)
			if (0 == (iter->second -= to_remove))
				levels.erase(iter);

		return FluidStack(id, to_remove);
	}

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction direction, bool remove, FluidAmount max_amount) {
		return extractFluid(direction, [](FluidID) { return true; }, remove, [=](FluidID) { return max_amount; });
	}

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction direction, bool remove) {
		return extractFluid(direction, remove, std::numeric_limits<FluidAmount>::max());
	}

	void FluidHoldingTileEntity::setFluidLevels(FluidContainer::Map map) {
		fluidContainer->levels = std::move(map);
		fluidsUpdated();
	}

	void FluidHoldingTileEntity::fluidsUpdated() {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		if (realm->getSide() == Side::Server) {
			increaseUpdateCounter();
			queueBroadcast();
		} else {
			getRealm()->getGame().toClient().signal_fluid_update().emit(std::dynamic_pointer_cast<HasFluids>(shared_from_this()));
		}
	}

	void FluidHoldingTileEntity::addObserver(const std::shared_ptr<Player> &player) {
		Observable::addObserver(player);
		player->send(TileEntityPacket(shared_from_this()));
		player->send(OpenFluidLevelsPacket(getGID()));
		player->queueForMove([this](const std::shared_ptr<Entity> &entity) {
			removeObserver(std::static_pointer_cast<Player>(entity));
			return true;
		});
	}

	void FluidHoldingTileEntity::toJSON(nlohmann::json &json) const {
		auto lock = const_cast<Lockable<FluidContainer::Map> &>(fluidContainer->levels).sharedLock();
		json["fluidLevels"] = fluidContainer->levels.getBase();
	}

	void FluidHoldingTileEntity::absorbJSON(Game &, const nlohmann::json &json) {
		fluidContainer->levels = json.at("fluidLevels").get<FluidContainer::Map>();
	}

	void FluidHoldingTileEntity::encode(Game &, Buffer &buffer) {
		HasFluids::encode(buffer);
	}

	void FluidHoldingTileEntity::decode(Game &, Buffer &buffer) {
		HasFluids::decode(buffer);
		fluidsUpdated();
	}

	void FluidHoldingTileEntity::broadcast() {
		if (forceBroadcast)
			TileEntity::broadcast();
		else
			broadcast(TileEntityPacket(shared_from_this()));
	}

	void FluidHoldingTileEntity::broadcast(const TileEntityPacket &packet) {
		assert(getSide() == Side::Server);
		auto lock = observers.uniqueLock();

		std::erase_if(observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return false;
		});
	}
}
