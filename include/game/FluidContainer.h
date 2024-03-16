#pragma once

#include "game/Container.h"
#include "game/Fluids.h"
#include "threading/Lockable.h"
#include "threading/LockableWeakPtr.h"

#include <cassert>
#include <map>

namespace Game3 {
	class HasFluids;

	class FluidContainer: public Container {
		public:
			using Map = std::map<FluidID, FluidAmount>;

			Lockable<Map> levels;
			LockableWeakPtr<HasFluids> weakOwner;

			FluidContainer(Map levels_ = {}, std::weak_ptr<HasFluids> weak_owner = {}):
				levels(std::move(levels_)), weakOwner(std::move(weak_owner)) {}

			inline std::shared_ptr<HasFluids> getOwner() const {
				auto out = weakOwner.lock();
				assert(out);
				return out;
			}
	};
}
