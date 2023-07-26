#pragma once

#include "game/Fluids.h"
#include "threading/Lockable.h"

#include <memory>
#include <map>

namespace Game3 {
	class Agent;
	class Buffer;
	class Game;
	class Inventory;

	class HasFluids {
		public:
			using Map = std::map<FluidID, FluidAmount>;

			HasFluids(Map = {});

			virtual size_t getMaxFluidTypes() const { return 1; }
			virtual FluidAmount getMaxLevel(FluidID) const;

			/** Returns how much fluid was unable to be added. */
			FluidAmount addFluid(FluidStack);

			virtual bool canInsertFluid(FluidStack);

			bool empty();

			virtual void fluidsUpdated() {}

			void encode(Buffer &);
			void decode(Buffer &);

		protected:
			Lockable<Map> fluidLevels;
	};
}
