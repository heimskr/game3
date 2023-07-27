#include "game/ClientGame.h"
#include "realm/Realm.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	FluidHoldingTileEntity::FluidHoldingTileEntity(HasFluids::Map map):
		HasFluids(std::move(map)) {}

	bool FluidHoldingTileEntity::canInsertFluid(FluidStack stack, Direction) {
		return canInsertFluid(stack);
	}

	FluidAmount FluidHoldingTileEntity::addFluid(FluidStack stack, Direction) {
		return addFluid(stack);
	}

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction, const std::function<bool(FluidID)> &predicate, bool remove, const std::function<FluidAmount(FluidID)> &max_amount) {
		{
			auto lock = fluidLevels.sharedLock();
			if (fluidLevels.empty())
				return std::nullopt;
		}

		auto lock = fluidLevels.uniqueLock();
		auto iter = fluidLevels.begin();

		// Advance through possible fluids until we find an acceptable one.
		if (predicate)
			while (iter != fluidLevels.end() && !predicate(iter->first))
				++iter;

		if (iter == fluidLevels.end())
			return std::nullopt;

		const FluidID id = iter->first;
		const FluidAmount to_remove = std::min(max_amount? max_amount(id) : std::numeric_limits<FluidAmount>::max(), iter->second);

		if (remove)
			if (0 == (iter->second -= to_remove))
				fluidLevels.erase(iter);

		return FluidStack(id, to_remove);
	}

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction direction, bool remove, FluidAmount max_amount) {
		return extractFluid(direction, [](FluidID) { return true; }, remove, [=](FluidID) { return max_amount; });
	}

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction direction, bool remove) {
		return extractFluid(direction, remove, std::numeric_limits<FluidAmount>::max());
	}

	void FluidHoldingTileEntity::setFluidLevels(HasFluids::Map map) {
		fluidLevels = std::move(map);
		fluidsUpdated();
	}

	void FluidHoldingTileEntity::fluidsUpdated() {
		auto realm = weakRealm.lock();
		if (!realm)
			return;

		if (realm->getSide() == Side::Server) {
			increaseUpdateCounter();
			broadcast();
		} else {
			getRealm()->getGame().toClient().signal_fluid_update().emit(std::dynamic_pointer_cast<HasFluids>(shared_from_this()));
		}
	}

	void FluidHoldingTileEntity::toJSON(nlohmann::json &json) const {
		auto lock = const_cast<Lockable<Map> &>(fluidLevels).sharedLock();
		json["fluidLevels"] = fluidLevels.getBase();
	}

	void FluidHoldingTileEntity::absorbJSON(Game &, const nlohmann::json &json) {
		fluidLevels = json.at("fluidLevels").get<Map>();
	}

	void FluidHoldingTileEntity::encode(Game &, Buffer &buffer) {
		HasFluids::encode(buffer);
	}

	void FluidHoldingTileEntity::decode(Game &, Buffer &buffer) {
		HasFluids::decode(buffer);
		fluidsUpdated();
	}
}
