#pragma once

#include "fluid/Fluid.h"

namespace Game3 {
	class SulfuricAcid: public Fluid {
		public:
			static Identifier ID() { return {"base", "fluid/sulfuric_acid"}; }

			SulfuricAcid(Fluid &&);

			void onCollision(const std::shared_ptr<LivingEntity> &) final;
	};
}
