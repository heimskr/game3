#pragma once

#include "fluid/Fluid.h"

namespace Game3 {
	class Lava: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/lava"}; }

			Lava(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
