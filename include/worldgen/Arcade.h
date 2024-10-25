#pragma once

#include <memory>
#include <random>

#include "types/Types.h"
#include "types/Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateArcade(const std::shared_ptr<Realm> &, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, Position entrance);
	}
}
