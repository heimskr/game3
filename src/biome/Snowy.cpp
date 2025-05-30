#include "graphics/Tileset.h"
#include "biome/Snowy.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	namespace {
		const std::unordered_set<Identifier> trees{"base:tile/winter_tree1", "base:tile/winter_tree2", "base:tile/winter_tree3"};
	}

	void Snowy::init(const std::shared_ptr<Realm> &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
		forestNoise.setSeed(-noise_seed * 3);
	}

	double Snowy::generate(Index row, Index column, std::default_random_engine &rng, const NoiseGenerator &, const WorldGenParams &params, double suggested_noise) {
		RealmPtr realm = getRealm();
		const auto wetness    = params.wetness;
		const auto stoneLevel = params.stoneLevel;

		static const Identifier sand        = "base:tile/sand";
		static const Identifier dark_ice    = "base:tile/dark_ice";
		static const Identifier light_ice   = "base:tile/light_ice";
		static const Identifier snow        = "base:tile/snow";
		static const Identifier stone       = "base:tile/stone";
		static const Identifier water       = "base:fluid/water";
		static const Identifier dirt        = "base:tile/dirt";
		static const Identifier light_grass = "base:tile/light_grass";
		static const Identifier grass       = "base:tile/grass";

		Position position{row, column};

		realm->setTile(Layer::Bedrock, position, stone, false);

		if (suggested_noise < wetness + 0.3) {
			realm->setTile(Layer::Soil, position, sand, false);
			realm->setFluid(position, water, params.getFluidLevel(suggested_noise, 0.3), true);
		} else if (suggested_noise < wetness + 0.39) {
			realm->setTile(Layer::Soil, position, sand, false);
		} else if (suggested_noise < wetness + 0.42) {
			realm->setTile(Layer::Soil, position, dirt, false);
			realm->setTile(Layer::Snow, position, dark_ice, false);
		} else if (suggested_noise < wetness + 0.5) {
			realm->setTile(Layer::Soil, position, dirt, false);
			realm->setTile(Layer::Vegetation, position, light_grass, false);
			realm->setTile(Layer::Snow, position, light_ice, false);
		} else if (stoneLevel < suggested_noise) {
			// Do nothing; there's stone on the bedrock layer already.
		} else {
			realm->setTile(Layer::Soil, position, dirt, false);
			realm->setTile(Layer::Vegetation, position, grass, false);
			realm->setTile(Layer::Snow, position, snow, false);

			const double forest_noise = forestNoise(row / params.noiseZoom, column / params.noiseZoom, 0.5);

			if (params.forestThreshold < forest_noise) {
				uint8_t mod = std::abs(column) % 2;

				std::default_random_engine tree_rng(static_cast<uint_fast32_t>(forest_noise * 1'000'000'000.));

				if (std::uniform_int_distribution(0, 99)(tree_rng) < 50) {
					mod = 1 - mod;
				}

				if ((std::abs(row) % 2) == mod) {
					realm->setTile(Layer::Submerged, position, choose(trees, rng), false);
				}
			}
		}

		return suggested_noise;
	}

	void Snowy::postgen(Index row, Index column, std::default_random_engine &, const NoiseGenerator &noisegen, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		constexpr double factor = 10;

		if (params.antiforestThreshold > noisegen(row / params.noiseZoom * factor, column / params.noiseZoom * factor, 0.)) {
			if (std::optional<TileID> tile = realm.tryTile(Layer::Submerged, {row, column}); tile && trees.contains(realm.getTileset()[*tile])) {
				realm.setTile(Layer::Submerged, {row, column}, 0, false);
			}
		}
	}
}
