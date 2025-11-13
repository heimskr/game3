#pragma once

#include "fluid/Fluid.h"

namespace Game3 {
	/** Water of indeterminate salinity. Produced by worldgen. */
	class Water: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/water"}; }

			Water(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) override;

			/** Converts indeterminate water to either seawater or fresh water at a given location using a flood fill algorithm.
			 *  The type is determined by the detected size of the water region. */
			FluidPtr resolve(const Place &, size_t count) override;
	};
}
