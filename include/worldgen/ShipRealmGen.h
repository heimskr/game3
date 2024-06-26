#pragma once

#include <memory>
#include <random>

#include "types/Types.h"

namespace Game3 {
	class Realm;
	class Ship;
	struct ChunkRange;
	struct WorldGenParams;

	namespace WorldGen {
		void generateShipRealmChunks(const std::shared_ptr<Realm> &, size_t noise_seed, const WorldGenParams &, const ChunkRange &, bool initial_generation, const std::shared_ptr<Ship> &parent);
	}
}
