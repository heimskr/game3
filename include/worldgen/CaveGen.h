#pragma once

#include "algorithm/NoiseGenerator.h"
#include "types/Types.h"

#include <memory>
#include <random>

namespace Game3 {
	class Realm;
	struct Position;
	struct ChunkRange;

	namespace WorldGen {
		void generateCave(const std::shared_ptr<Realm> &, std::default_random_engine &, int noise_seed, const ChunkRange &range);
		bool generateNormalCaveTile(const std::shared_ptr<Realm> &, Index row, Index column, const NoiseGenerator &);
		bool generateGrimCaveTile(const std::shared_ptr<Realm> &, Index row, Index column, const NoiseGenerator &);
		bool generateCaveTile(const std::shared_ptr<Realm> &, Index row, Index column, const NoiseGenerator &);
		void generateCaveFull(const std::shared_ptr<Realm> &, std::default_random_engine &, int noise_seed, const Position &exit_position, Position &entrance, RealmID parent_realm, const ChunkRange &range);
	}
}
