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
			const FluidAmount remainder = to_add - (max - level);
			level = max;
			lock.unlock();
			fluidsUpdated();
			return remainder;
		}

		if (max < level + to_add) {
			const FluidAmount remainder = level + to_add - max;
			level = max;
			lock.unlock();
			fluidsUpdated();
			return remainder;
		}

		level += to_add;
		lock.unlock();
		fluidsUpdated();
		return 0;
	}

	bool HasFluids::canInsertFluid(FluidStack stack) {
		auto lock = fluidLevels.sharedLock();

		auto iter = fluidLevels.find(stack.id);

		if (iter == fluidLevels.end())
			return fluidLevels.size() < getMaxFluidTypes() && stack.amount <= getMaxLevel(stack.id);

		const FluidAmount current_amount = iter->second;

		// Integer overflow definitely isn't allowed.
		if (current_amount + stack.amount < current_amount)
			return false;

		return current_amount + stack.amount <= getMaxLevel(stack.id);
	}

	bool HasFluids::fluidsEmpty() {
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
