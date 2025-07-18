#pragma once

#include "fluid/Fluid.h"

namespace Game3 {
	class Brine: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/brine"}; }

			Brine(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
