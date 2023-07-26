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

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction, bool remove, FluidAmount max_amount) {
		{
			auto lock = fluidLevels.sharedLock();
			if (fluidLevels.empty())
				return std::nullopt;
		}

		auto lock = fluidLevels.uniqueLock();
		auto iter = fluidLevels.begin();

		if (iter == fluidLevels.end())
			return std::nullopt;

		const FluidID id = iter->first;
		const FluidAmount to_remove = std::min(max_amount, iter->second);

		if (remove)
			if (0 == (iter->second -= to_remove))
				fluidLevels.erase(iter);

		return FluidStack(id, to_remove);
	}

	std::optional<FluidStack> FluidHoldingTileEntity::extractFluid(Direction direction, bool remove) {
		return extractFluid(direction, remove, std::numeric_limits<FluidAmount>::max());
	}

	void FluidHoldingTileEntity::setFluidLevels(HasFluids::Map map) {
		fluidLevels = std::move(map);
		fluidsUpdated();
	}

	void FluidHoldingTileEntity::fluidsUpdated() {
		increaseUpdateCounter();
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
	}
}
