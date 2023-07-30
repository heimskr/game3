#include "game/Game.h"
#include "game/HasFluids.h"
#include "net/Buffer.h"

namespace Game3 {
	HasFluids::HasFluids(std::shared_ptr<FluidContainer> fluid_container):
		fluidContainer(std::move(fluid_container)) {}

	FluidAmount HasFluids::getMaxLevel(FluidID) {
		return std::numeric_limits<FluidLevel>::max();
	}

	FluidAmount HasFluids::addFluid(FluidStack stack) {
		assert(fluidContainer);

		auto [id, to_add] = stack;

		auto &levels = fluidContainer->levels;
		auto lock = levels.uniqueLock();

		if (getMaxFluidTypes() <= levels.size() && !levels.contains(id))
			return to_add;

		FluidAmount &level = levels[id];
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
		assert(fluidContainer);

		auto &levels = fluidContainer->levels;
		auto lock = levels.sharedLock();

		auto iter = levels.find(stack.id);

		if (iter == levels.end())
			return levels.size() < getMaxFluidTypes() && stack.amount <= getMaxLevel(stack.id);

		const FluidAmount current_amount = iter->second;

		// Integer overflow definitely isn't allowed.
		if (current_amount + stack.amount < current_amount)
			return false;

		return current_amount + stack.amount <= getMaxLevel(stack.id);
	}

	bool HasFluids::fluidsEmpty() {
		assert(fluidContainer);
		auto lock = fluidContainer->levels.sharedLock();
		return fluidContainer->levels.empty();
	}

	void HasFluids::encode(Buffer &buffer) {
		assert(fluidContainer);
		buffer << fluidContainer->levels;
	}

	void HasFluids::decode(Buffer &buffer) {
		assert(fluidContainer);
		buffer >> fluidContainer->levels;
	}
}
