#pragma once

#include <memory>
#include <random>

#include "types/Types.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateCarpet(const std::shared_ptr<Realm> &, std::default_random_engine &, Index width, Index height, int padding = -1, Layer layer = Layer::Submerged);
	}
}
