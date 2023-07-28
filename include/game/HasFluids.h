#pragma once

#include "game/FluidContainer.h"
#include "threading/Lockable.h"

#include <memory>
#include <map>

namespace Game3 {
	class Agent;
	class Buffer;
	class FluidContainer;
	class Game;
	class Inventory;

	class HasFluids {
		public:
			std::shared_ptr<FluidContainer> fluidContainer;

			HasFluids(std::shared_ptr<FluidContainer> = nullptr);

			virtual size_t getMaxFluidTypes() const { return 1; }
			virtual FluidAmount getMaxLevel(FluidID) const;

			/** Returns how much fluid was unable to be added. */
			virtual FluidAmount addFluid(FluidStack);

			virtual bool canInsertFluid(FluidStack);

			bool fluidsEmpty();

			virtual void fluidsUpdated() {}

			void encode(Buffer &);
			void decode(Buffer &);

	};
}
