#pragma once

#include "game/Fluids.h"
#include "threading/Lockable.h"

#include <memory>
#include <unordered_map>

namespace Game3 {
	class Agent;
	class Buffer;
	class Game;
	class Inventory;

	class HasFluids {
		public:
			using Map = std::unordered_map<FluidID, FullFluidLevel>;

			HasFluids(Map = {});

			Lockable<Map> fluidLevels;

			virtual size_t getMaxFluidTypes() const { return 1; }
			virtual FullFluidLevel getMaxLevel(FluidID) const;

			/** Returns how much fluid was unable to be added. */
			virtual FullFluidLevel addFluid(FluidStack);

			virtual void fluidsUpdated() {}

			void encode(Buffer &);
			void decode(Buffer &);
	};
}
