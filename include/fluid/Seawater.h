#pragma once

#include "fluid/Water.h"

namespace Game3 {
	class Seawater: public Water {
		public:
			static Identifier ID() { return {"base", "fluid/seawater"}; }

			using Water::Water;

			FluidPtr resolve(const Place &, size_t) override { return nullptr; }
	};
}
