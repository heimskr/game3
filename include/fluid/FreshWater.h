#pragma once

#include "fluid/Water.h"

namespace Game3 {
	class FreshWater: public Water {
		public:
			static Identifier ID() { return {"base", "fluid/fresh_water"}; }

			using Water::Water;

			FluidPtr resolve(const Place &, size_t) override { return nullptr; }
	};
}
