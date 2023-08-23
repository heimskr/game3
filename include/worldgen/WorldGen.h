#pragma once

#include <functional>
#include <memory>
#include <random>

#include <nlohmann/json_fwd.hpp>

#include "Types.h"
#include "game/ChunkPosition.h"
#include "threading/ThreadPool.h"

namespace Game3 {
	class Realm;
	struct Position;

	using BuildingGenerator = std::function<void(const std::shared_ptr<Realm> &, std::default_random_engine &, const std::shared_ptr<Realm> &, Index width, Index height, const Position &)>;

	struct WorldGenParams {
		double wetness = -.15;
		double stoneLevel = 0.8;
		double forestThreshold = 0.5;
		double antiforestThreshold = -0.4;
		double biomeZoom = 1000.;
	};

	void from_json(const nlohmann::json &, WorldGenParams &);
	void to_json(nlohmann::json &, const WorldGenParams &);

	namespace WorldGen {
		extern ThreadPool pool;
	}
}
