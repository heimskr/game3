#pragma once

#include <functional>
#include <memory>
#include <random>

#include "game/ChunkPosition.h"

namespace Game3 {
	class Realm;
	struct Position;

	using BuildingGenerator = std::function<void(const std::shared_ptr<Realm> &, std::default_random_engine &, const std::shared_ptr<Realm> &, const Position &)>;

	struct WorldGenParams {
		double wetness = -.15;
		double stoneLevel = 0.8;
		double forestThreshold = 0.5;
		double antiforestThreshold = -0.4;
		double biomeZoom = 1000.;
		ChunkRange range;
	};
}
