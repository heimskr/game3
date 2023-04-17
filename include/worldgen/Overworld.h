#pragma once

#include <memory>
#include <random>

#include "Types.h"

namespace Game3 {
	class Realm;
	struct WorldGenParams;

	namespace WorldGen {
		void generateOverworld(const std::shared_ptr<Realm> &, size_t noise_seed, const WorldGenParams &);
	}
}
