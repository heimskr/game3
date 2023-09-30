#pragma once

#include <memory>
#include <random>

#include "Types.h"
#include "lib/noise.h"

namespace Game3 {
	class Realm;
	struct Position;
	struct ChunkRange;

	namespace WorldGen {
		void generateCave(const std::shared_ptr<Realm> &, std::default_random_engine &, int noise_seed, const ChunkRange &range);
		bool generateNormalCaveTile(const std::shared_ptr<Realm> &, Index row, Index column, const noise::module::Perlin &);
		bool generateGrimCaveTile(const std::shared_ptr<Realm> &, Index row, Index column, const noise::module::Perlin &);
		bool generateCaveTile(const std::shared_ptr<Realm> &, Index row, Index column, const noise::module::Perlin &);
		void generateCaveFull(const std::shared_ptr<Realm> &, std::default_random_engine &, int noise_seed, const Position &exit_position, Position &entrance, RealmID parent_realm, const ChunkRange &range);
	}
}
