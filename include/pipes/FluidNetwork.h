#pragma once

#include "game/HasFluids.h"
#include "pipes/PipeNetwork.h"

namespace Game3 {
	class Inventory;
	struct FluidStack;

	class FluidNetwork: public PipeNetwork, private HasFluids {
		public:
			FluidNetwork(size_t id_, const std::shared_ptr<Realm> &);

			PipeType getType() const final { return PipeType::Fluid; }

			void tick(Tick) final;
			bool canWorkWith(const std::shared_ptr<TileEntity> &) const final;

		private:
			/** Returns the amount not distributed. */
			FluidAmount distribute(const FluidStack &stack);

			size_t getMaxFluidTypes() const final { return std::numeric_limits<size_t>::max(); }
	};
}
