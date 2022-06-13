#pragma once

#include <memory>
#include <random>

#include "Types.h"
#include "Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateTown(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, Index index, Index width, Index height, Index pad);
	}
}
