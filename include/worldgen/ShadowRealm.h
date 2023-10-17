#pragma once

#include <memory>
#include <random>

#include "types/Types.h"

namespace Game3 {
	class Realm;
	struct ChunkRange;
	struct WorldGenParams;

	namespace WorldGen {
		void generateShadowRealm(const std::shared_ptr<Realm> &, size_t noise_seed, const WorldGenParams &, const ChunkRange &, bool initial_generation);
	}
}
