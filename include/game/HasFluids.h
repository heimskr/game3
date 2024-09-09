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

	/** None of these methods should (or do) lock anything. */
	class HasFluids {
		public:
			std::shared_ptr<FluidContainer> fluidContainer;

			HasFluids() = default;
			explicit HasFluids(std::shared_ptr<FluidContainer>);

			virtual ~HasFluids() = default;

			/** Sets the owner of the fluid container. */
			void init(const std::shared_ptr<HasFluids> &self);

			virtual size_t getMaxFluidTypes() const { return 1; }
			virtual FluidAmount getMaxLevel(FluidID);

			/** Returns how much fluid was unable to be added. */
			virtual FluidAmount addFluid(FluidStack);

			virtual bool canInsertFluid(FluidStack);

			virtual FluidAmount fluidInsertable(FluidID);

			bool fluidsEmpty();

			virtual void fluidsUpdated();

			void encode(Buffer &);
			void decode(Buffer &);

		protected:
			virtual std::shared_ptr<Game> getGame() const = 0;
	};
}
