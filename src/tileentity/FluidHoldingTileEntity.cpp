#include "game/ClientGame.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "ui/module/FluidLevelsModule.h"
#include "util/Cast.h"

namespace Game3 {
	FluidHoldingTileEntity::FluidHoldingTileEntity(FluidContainer::Map map):
		HasFluids(std::make_shared<FluidContainer>(std::move(map))) {}

	bool FluidHoldingTileEntity::canInsertFluid(FluidStack stack, Direction) {
		return canInsertFluid(stack);
	}

	FluidAmount FluidHoldingTileEntity::addFluid(FluidStack stack, Direction) {
		auto lock = fluidContainer->levels.uniqueLock();
		return addFluid(stack);
	}

	FluidAmount FluidHoldingTileEntity::fluidInsertable(FluidID id, Direction) {
		auto lock = fluidContainer->levels.sharedLock();
		return HasFluids::fluidInsertable(id);
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
			getRealm()->getGame().toClient().signalFluidUpdate().emit(std::dynamic_pointer_cast<HasFluids>(shared_from_this()));
		}
	}

	void FluidHoldingTileEntity::addObserver(const PlayerPtr &player, bool silent) {
		Observable::addObserver(player, silent);

		player->send(TileEntityPacket(getSelf()));

		if (!silent)
			player->send(OpenModuleForAgentPacket(FluidLevelsModule::ID(), getGID(), true));

		player->queueForMove([weak_self = getWeakSelf()](const EntityPtr &entity, bool) {
			if (auto self = weak_self.lock())
				safeDynamicCast<FluidHoldingTileEntity>(self)->removeObserver(safeDynamicCast<Player>(entity));
			return true;
		});
	}

	void FluidHoldingTileEntity::toJSON(nlohmann::json &json) const {
		auto lock = fluidContainer->levels.sharedLock();
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

	void FluidHoldingTileEntity::broadcast(bool force) {
		if (force)
			TileEntity::broadcast(true);
		else
			broadcast(TileEntityPacket(getSelf()));
	}

	void FluidHoldingTileEntity::broadcast(const TileEntityPacket &packet) {
		assert(getSide() == Side::Server);
		auto lock = observers.uniqueLock();

		std::erase_if(observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});
	}
}
