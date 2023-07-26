#include "game/Game.h"
#include "game/HasFluids.h"
#include "net/Buffer.h"

namespace Game3 {
	HasFluids::HasFluids(Map fluid_levels):
		fluidLevels(std::move(fluid_levels)) {}

	FluidAmount HasFluids::getMaxLevel(FluidID) const {
		return std::numeric_limits<FluidLevel>::max();
	}

	FluidAmount HasFluids::addFluid(FluidStack stack) {
		auto [id, to_add] = stack;
		auto lock = fluidLevels.uniqueLock();

		if (getMaxFluidTypes() <= fluidLevels.size() && !fluidLevels.contains(id))
			return to_add;

		FluidAmount &level = fluidLevels[id];
		const FluidAmount max = getMaxLevel(id);

		// Just in case there would be integer overflow.
		if (level + to_add < level) {
			const FluidAmount remainder = to_add - (std::numeric_limits<FluidAmount>::max() - level);
			level = max;
			fluidsUpdated();
			return remainder;
		}

		if (max < level + to_add) {
			const FluidAmount remainder = level + to_add - max;
			level = max;
			fluidsUpdated();
			return remainder;
		}

		level += to_add;
		fluidsUpdated();
		return 0;
	}

	bool HasFluids::canInsertFluid(FluidStack stack) {
		auto lock = fluidLevels.sharedLock();

		auto iter = fluidLevels.find(stack.id);

		if (iter == fluidLevels.end())
			return fluidLevels.size() < getMaxFluidTypes() && stack.level <= getMaxLevel(stack.id);

		const FluidAmount current_level = iter->second;

		// Integer overflow definitely isn't allowed.
		if (current_level + stack.level < current_level)
			return false;

		return current_level + stack.level <= getMaxLevel(stack.id);
	}

	bool HasFluids::empty() {
		auto lock = fluidLevels.sharedLock();
		return fluidLevels.empty();
	}

	void HasFluids::encode(Buffer &buffer) {
		buffer << fluidLevels;
	}

	void HasFluids::decode(Buffer &buffer) {
		buffer >> fluidLevels;
	}
}
