#pragma once

#include <memory>
#include <random>

#include "Types.h"
#include "Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateTown(const std::shared_ptr<Realm> &, std::default_random_engine &, const Position &, Index width, Index height, Index pad, int seed);
	}
}
