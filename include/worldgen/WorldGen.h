#pragma once

#include <functional>
#include <memory>
#include <random>

#include <boost/json/fwd.hpp>

#include "types/Types.h"
#include "types/ChunkPosition.h"
#include "game/Fluids.h"
#include "threading/ThreadPool.h"

namespace Game3 {
	class Realm;
	struct Position;

	using BuildingGenerator = std::function<void(const std::shared_ptr<Realm> &, std::default_random_engine &, const std::shared_ptr<Realm> &, Index width, Index height, const Position &)>;

	double getDefaultWetness();

	struct WorldGenParams {
		double wetness = getDefaultWetness();
		double stoneLevel = 1.2;
		double forestThreshold = 0.5;
		double antiforestThreshold = -0.4;
		double biomeZoom = 1000.;
		double noiseZoom = 150.;

		/** noise ∈ [-1, 1] */
		FluidLevel getFluidLevel(double noise, double threshold = 0.3) const {
			const double level = (1. + noise) / ((1. + threshold) + wetness);
			return FluidTile::FULL * std::max(0., std::cos(level * M_PI / 4.));
		}
	};

	WorldGenParams tag_invoke(boost::json::value_to_tag<WorldGenParams>, const boost::json::value &);
	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const WorldGenParams &);

	namespace WorldGen {
		extern ThreadPool pool;
	}
}
