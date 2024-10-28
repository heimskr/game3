#pragma once

#include <map>
#include <memory>
#include <random>

#include "types/Types.h"

namespace Game3 {
	class NoiseGenerator;
	class Realm;
	struct WorldGenParams;

	class Biome {
		public:
			constexpr static BiomeType VOID      = 0;
			constexpr static BiomeType GRASSLAND = 1;
			constexpr static BiomeType VOLCANIC  = 2;
			constexpr static BiomeType SNOWY     = 3;
			constexpr static BiomeType DESERT    = 4;
			constexpr static BiomeType CAVE      = 5;
			constexpr static BiomeType GRIMSTONE = 6;
			constexpr static BiomeType SHIP      = 7;
			constexpr static BiomeType COUNT     = SHIP + 1;

			BiomeType type;

			Biome() = delete;
			Biome(BiomeType type_): type(type_) {}

			Biome(const Biome &) = default;
			Biome(Biome &&) noexcept = default;

			virtual ~Biome() = default;

			Biome & operator=(const Biome &) = default;
			Biome & operator=(Biome &&) noexcept = default;

			virtual void init(const std::shared_ptr<Realm> &, int noise_seed);

			/** Returns the noise value generated for the position. */
			virtual double generate(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &, double suggested_noise);

			virtual void postgen(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &) {
				(void) row; (void) column;
			}

			static std::map<BiomeType, std::shared_ptr<Biome>> getMap(const std::shared_ptr<Realm> &, int noise_seed);

		protected:
			std::shared_ptr<Realm> getRealm() const;
			void setRealm(const std::shared_ptr<Realm> &);
			virtual std::shared_ptr<Biome> clone() const { return std::make_shared<Biome>(*this); }

		private:
			std::weak_ptr<Realm> weakRealm;
			static const std::map<BiomeType, std::shared_ptr<const Biome>> & getMap();
	};

	using BiomePtr = std::shared_ptr<Biome>;
}