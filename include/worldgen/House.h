#pragma once

#include <memory>
#include <random>

#include "types/Types.h"
#include "types/Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		/** Requires the town's keep to be generated first. */
		void generateHouse(const std::shared_ptr<Realm> &realm, std::default_random_engine &, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, Position entrance);
	}
}
