#include "graphics/Tileset.h"
#include "biome/Desert.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	namespace {
		const std::unordered_set<Identifier> cactuses {
			"base:tile/cactus1"_id,
			"base:tile/cactus2"_id,
			"base:tile/cactus3"_id,
			"base:tile/cactus4"_id,
		};
	}

	void Desert::init(const std::shared_ptr<Realm> &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
		forestNoise.setSeed(-noise_seed * 3);
	}

	double Desert::generate(Index row, Index column, std::default_random_engine &rng, const NoiseGenerator &, const WorldGenParams &params, double suggested_noise) {
		RealmPtr realm = getRealm();
		const auto wetness    = params.wetness;
		const auto stoneLevel = params.stoneLevel;

		static const Identifier sand  = "base:tile/sand"_id;
		static const Identifier stone = "base:tile/stone"_id;
		static const Identifier water = "base:fluid/water"_id;

		Position position{row, column};

		realm->setTile(Layer::Bedrock, position, stone, false);

		if (suggested_noise < wetness + 0.3) {
			realm->setTile(Layer::Soil, position, sand, false);
			realm->setFluid(position, water, params.getFluidLevel(suggested_noise, 0.3), true);
		} else if (suggested_noise < wetness + 0.4) {
			realm->setTile(Layer::Soil, position, sand, false);
		} else if (stoneLevel < suggested_noise) {
			// Do nothing; there's stone on the bedrock layer already.
		} else {
			realm->setTile(Layer::Soil, position, sand, false);
			const double forest_noise = forestNoise(row / params.noiseZoom, column / params.noiseZoom, 0.5);
			if (params.forestThreshold - 0.2 < forest_noise) {
				std::default_random_engine tree_rng(static_cast<uint_fast32_t>(forest_noise * 1'000'000'000.));
				std::uniform_int_distribution hundred{0, 99};
				if (hundred(tree_rng) < 75) {
					return suggested_noise;
				}
				uint8_t mod = abs(column) % 2;
				if (hundred(tree_rng) < 50) {
					mod = 1 - mod;
				}
				if ((abs(row) % 2) == mod) {
					realm->setTile(Layer::Submerged, position, choose(cactuses, rng), false);
				}
			}
		}

		return suggested_noise;
	}

	void Desert::postgen(Index row, Index column, std::default_random_engine &, const NoiseGenerator &noisegen, const WorldGenParams &params) {
		RealmPtr realm = getRealm();
		constexpr double factor = 10;

		if (params.antiforestThreshold > noisegen(row / params.noiseZoom * factor, column / params.noiseZoom * factor, 0.)) {
			if (std::optional<TileID> tile = realm->tryTile(Layer::Submerged, {row, column}); tile && cactuses.contains(realm->getTileset()[*tile])) {
				realm->setTile(Layer::Submerged, {row, column}, 0, false);
			}
		}
	}
}
