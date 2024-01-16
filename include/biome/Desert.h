#pragma once

#include "algorithm/NoiseGenerator.h"
#include "biome/Biome.h"

namespace Game3 {
	class Desert: public Biome {
		public:
			constexpr static double THRESHOLD = -0.15;

			Desert(): Biome(Biome::DESERT) {}

			void init(Realm &, int noise_seed) override;
			double generate(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &) override;
			void postgen(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &) override;

		protected:
			std::shared_ptr<Biome> clone() const override { return std::make_shared<Desert>(*this); }

		private:
			DefaultNoiseGenerator forestNoise;
	};
}
