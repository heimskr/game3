#pragma once

#include <memory>
#include <random>

#include "types/Types.h"
#include "types/Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateTavern(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, const Position &entrance);
	}
}
