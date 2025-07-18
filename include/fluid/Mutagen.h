#pragma once

#include "fluid/Fluid.h"

namespace Game3 {
	class Mutagen: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/mutagen"}; }

			Mutagen(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
