#pragma once

#include "game/Container.h"
#include "game/Fluids.h"
#include "threading/Lockable.h"

namespace Game3 {
	class FluidContainer: public Container {
		public:
			using Map = std::map<FluidID, FluidAmount>;

			Lockable<Map> levels;

			FluidContainer(Map levels_ = {}):
				levels(std::move(levels_)) {}
	};
}
