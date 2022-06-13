#pragma once

#include <memory>
#include <random>

#include "Types.h"
#include "Position.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateHouse(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, RealmID parent_realm, const Position &entrance);
	}
}
