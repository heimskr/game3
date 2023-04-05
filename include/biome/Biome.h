#pragma once

#include <map>
#include <memory>
#include <random>

#include "Types.h"

namespace noise::module {
	class Perlin;
}

namespace Game3 {
	class Realm;

	class Biome {
		public:
			constexpr static BiomeType VOID_ID        = 0;
			constexpr static BiomeType GRASSLAND_ID   = 1;
			constexpr static BiomeType VOLCANIC_ID    = 2;
			constexpr static BiomeType SNOWY_ID       = 3;
			constexpr static BiomeType DESERT_ID      = 4;
			constexpr static BiomeType CAVE_ID        = 5;
			constexpr static BiomeType COUNT          = CAVE_ID + 1;

			constexpr static double NOISE_ZOOM = 100.;

			const BiomeType type;

			Biome() = delete;
			Biome(BiomeType type_): type(type_) {}

			virtual void init(Realm &realm_, int noise_seed, const std::shared_ptr<double[]> &saved_noise);

			virtual void generate(Index row, Index column, std::default_random_engine &, const noise::module::Perlin &) {
				(void) row; (void) column;
			}

			virtual void postgen(Index row, Index column, std::default_random_engine &, const noise::module::Perlin &) {
				(void) row; (void) column;
			}

			static std::map<BiomeType, std::shared_ptr<Biome>> getMap(Realm &, int noise_seed, const std::shared_ptr<double[]> &saved_noise);

		protected:
			inline Realm * getRealm() { return realm; }
			inline void setRealm(Realm &realm_) { realm = &realm_; }
			virtual std::shared_ptr<Biome> clone() const { return std::make_shared<Biome>(*this); }
			std::shared_ptr<double[]> savedNoise;

		private:
			Realm *realm = nullptr;
			static std::map<BiomeType, std::shared_ptr<const Biome>> map;
	};

	using BiomePtr = std::shared_ptr<Biome>;
}