#pragma once

#include "algorithm/NoiseGenerator.h"
#include "biome/Biome.h"

namespace Game3 {
	class Grassland: public Biome {
		public:
			constexpr static double THRESHOLD = -0.15;

			Grassland(): Biome(Biome::GRASSLAND) {}

			void init(const std::shared_ptr<Realm> &, int noise_seed) override;
			double generate(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &, double suggested_noise) override;
			void postgen(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &) override;

		protected:
			std::shared_ptr<Biome> clone() const override { return std::make_shared<Grassland>(*this); }

		private:
			DefaultNoiseGenerator forestNoise;
			FluidID water = -1;
	};
}
