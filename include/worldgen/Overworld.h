#pragma once

#include <memory>
#include <random>

#include "Types.h"

namespace Game3 {
	class Realm;

	namespace WorldGen {
		void generateOverworld(const std::shared_ptr<Realm> &, std::default_random_engine &, int noise_seed, double noise_zoom, double noise_threshold);
	}
}
