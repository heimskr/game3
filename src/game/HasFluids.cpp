#include "game/Game.h"
#include "game/HasFluids.h"
#include "net/Buffer.h"

namespace Game3 {
	HasFluids::HasFluids(std::shared_ptr<FluidContainer> fluid_container):
		fluidContainer(std::move(fluid_container)) {}

	void HasFluids::init(const std::shared_ptr<HasFluids> &self) {
		if (fluidContainer) {
			fluidContainer->weakOwner = self;
		}
	}

	FluidAmount HasFluids::getMaxLevel(FluidID) {
		return std::numeric_limits<FluidLevel>::max();
	}

	FluidAmount HasFluids::addFluid(FluidStack stack) {
		assert(fluidContainer);

		auto [id, to_add] = stack;
		auto &levels = fluidContainer->levels;

		if (getMaxFluidTypes() <= levels.size() && !levels.contains(id)) {
			return to_add;
		}

		FluidAmount &level = levels[id];
		const FluidAmount max = getMaxLevel(id);

		// Just in case there would be integer overflow.
		if (level + to_add < level) {
			const FluidAmount remainder = to_add - (max - level);
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
		assert(fluidContainer);

		auto &levels = fluidContainer->levels;

		auto iter = levels.find(stack.id);

		if (iter == levels.end()) {
			return levels.size() < getMaxFluidTypes() && stack.amount <= getMaxLevel(stack.id);
		}

		const FluidAmount current_amount = iter->second;

		// Integer overflow definitely isn't allowed.
		if (current_amount + stack.amount < current_amount) {
			return false;
		}

		return current_amount + stack.amount <= getMaxLevel(stack.id);
	}

	FluidAmount HasFluids::fluidInsertable(FluidID id) {
		assert(fluidContainer);
		auto &levels = fluidContainer->levels;

		auto iter = levels.find(id);
		if (iter == levels.end()) {
			return levels.size() < getMaxFluidTypes()? getMaxLevel(id) : 0;
		}

		return getMaxLevel(id) - iter->second;
	}

	bool HasFluids::fluidsEmpty() {
		assert(fluidContainer);
		return fluidContainer->levels.empty();
	}

	void HasFluids::fluidsUpdated() {
		fluidContainer->increaseUpdateCounter();
	}

	void HasFluids::encode(Buffer &buffer) {
		assert(fluidContainer);
		buffer << fluidContainer->levels;
	}

	void HasFluids::decode(BasicBuffer &buffer) {
		assert(fluidContainer);
		buffer >> fluidContainer->levels;
		fluidsUpdated();
	}
}
