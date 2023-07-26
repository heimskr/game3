#include "game/Game.h"
#include "game/HasFluids.h"
#include "net/Buffer.h"

namespace Game3 {
	HasFluids::HasFluids(Map fluid_levels):
		fluidLevels(std::move(fluid_levels)) {}

	FullFluidLevel HasFluids::getMaxLevel(FluidID) const {
		return std::numeric_limits<FluidLevel>::max();
	}

	FullFluidLevel HasFluids::addFluid(FluidStack stack) {
		auto [id, to_add] = stack;
		auto lock = fluidLevels.uniqueLock();

		if (getMaxFluidTypes() <= fluidLevels.size() && !fluidLevels.contains(id))
			return to_add;

		FullFluidLevel &level = fluidLevels[id];
		const FullFluidLevel max = getMaxLevel(id);

		// Just in case there would be integer overflow.
		if (level + to_add < level) {
			const FullFluidLevel remainder = to_add - (std::numeric_limits<FullFluidLevel>::max() - level);
			level = max;
			fluidsUpdated();
			return remainder;
		}

		if (max < level + to_add) {
			const FullFluidLevel remainder = level + to_add - max;
			level = max;
			fluidsUpdated();
			return remainder;
		}

		level += to_add;
		fluidsUpdated();
		return 0;
	}

	void HasFluids::encode(Buffer &buffer) {
		buffer << fluidLevels;
	}

	void HasFluids::decode(Buffer &buffer) {
		buffer >> fluidLevels;
	}
}
