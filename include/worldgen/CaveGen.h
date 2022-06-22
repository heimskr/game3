#pragma once

#include <memory>
#include <random>

#include "Types.h"

namespace Game3 {
	class Realm;
	struct Position;

	namespace WorldGen {
		void generateCave(const std::shared_ptr<Realm> &, std::default_random_engine &, int noise_seed, Index exit_index, Position &entrance, RealmID parent_realm);
	}
}
