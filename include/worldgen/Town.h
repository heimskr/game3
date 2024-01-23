#pragma once

#include <memory>
#include <random>

#include "types/Types.h"
#include "types/Position.h"

namespace Game3 {
	class Realm;
	class Village;

	namespace WorldGen {
		void generateTown(const std::shared_ptr<Realm> &, std::default_random_engine &, const Position &, Index width, Index height, Index pad, int seed, const std::shared_ptr<Village> &);
	}
}
