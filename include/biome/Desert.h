#pragma once

#include "biome/Biome.h"
#include "lib/noise.h"

namespace Game3 {
	class Desert: public Biome {
		public:
			constexpr static double THRESHOLD = -0.15;

			Desert(): Biome(Biome::DESERT) {}

			void init(Realm &, int noise_seed, const std::shared_ptr<double[]> &shared_noise) override;
			double generate(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &, const WorldGenParams &) override;
			void postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &, const WorldGenParams &) override;

		protected:
			std::shared_ptr<Biome> clone() const override { return std::make_shared<Desert>(*this); }

		private:
			std::shared_ptr<noise::module::Perlin> forestPerlin;
	};
}
