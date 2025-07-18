#pragma once

#include "fluid/Fluid.h"

namespace Game3 {
	class PowderedSnow: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/powdered_snow"}; }

			PowderedSnow(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
