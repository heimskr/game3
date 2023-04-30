#pragma once

#include <memory>
#include <random>

#include "Types.h"

namespace Game3 {
	class Realm;
	struct Position;
	struct ChunkRange;

	namespace WorldGen {
		void generateCave(const std::shared_ptr<Realm> &, std::default_random_engine &, int noise_seed, const Position &exit_position, Position &entrance, RealmID parent_realm, const ChunkRange &range);
	}
}
