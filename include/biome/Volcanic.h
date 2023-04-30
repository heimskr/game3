#pragma once

#include "biome/Biome.h"
#include "lib/noise.h"

namespace Game3 {
	class Volcanic: public Biome {
		public:
			constexpr static double THRESHOLD = -0.15;

			Volcanic(): Biome(Biome::VOLCANIC) {}

			void init(Realm &, int noise_seed, const std::shared_ptr<double[]> &shared_noise) override;
			double generate(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &, const WorldGenParams &) override;
			void postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &, const WorldGenParams &) override;

		protected:
			std::shared_ptr<Biome> clone() const override { return std::make_shared<Volcanic>(*this); }
	};
}
