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

	void Snowy::init(Realm &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
		forestNoise.setSeed(-noise_seed * 3);
	}

	double Snowy::generate(Index row, Index column, std::default_random_engine &rng, const NoiseGenerator &, const WorldGenParams &params, double suggested_noise) {
		Realm &realm = *getRealm();
		const auto wetness    = params.wetness;
		const auto stoneLevel = params.stoneLevel;

		static const Identifier sand          = "base:tile/sand";
		static const Identifier dark_ice      = "base:tile/dark_ice";
		static const Identifier light_ice     = "base:tile/light_ice";
		static const Identifier snow          = "base:tile/snow";
		static const Identifier stone         = "base:tile/stone";
		static const Identifier water_fluid   = "base:fluid/water";

		if (suggested_noise < wetness + 0.3) {
			realm.setTile(Layer::Terrain, {row, column}, sand, false);
			realm.setFluid({row, column}, water_fluid, params.getFluidLevel(suggested_noise, 0.3), true);
		} else if (suggested_noise < wetness + 0.39) {
			realm.setTile(Layer::Terrain, {row, column}, sand, false);
		} else if (suggested_noise < wetness + 0.42) {
			realm.setTile(Layer::Terrain, {row, column}, dark_ice, false);
		} else if (suggested_noise < wetness + 0.5) {
			realm.setTile(Layer::Terrain, {row, column}, light_ice, false);
		} else if (stoneLevel < suggested_noise) {
			realm.setTile(Layer::Terrain, {row, column}, stone, false);
		} else {
			realm.setTile(Layer::Terrain, {row, column}, snow, false);
			const double forest_noise = forestNoise(row / params.noiseZoom, column / params.noiseZoom, 0.5);
			if (params.forestThreshold < forest_noise) {
				uint8_t mod = abs(column) % 2;
				std::default_random_engine tree_rng(static_cast<uint_fast32_t>(forest_noise * 1'000'000'000.));
				if (std::uniform_int_distribution(0, 99)(tree_rng) < 50)
					mod = 1 - mod;
				if ((abs(row) % 2) == mod)
					realm.setTile(Layer::Submerged, {row, column}, choose(trees, rng), false);
			}
		}

		return suggested_noise;
	}

	void Snowy::postgen(Index row, Index column, std::default_random_engine &, const NoiseGenerator &noisegen, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		constexpr double factor = 10;

		if (params.antiforestThreshold > noisegen(row / params.noiseZoom * factor, column / params.noiseZoom * factor, 0.))
			if (auto tile = realm.tryTile(Layer::Submerged, {row, column}); tile && trees.contains(realm.getTileset()[*tile]))
				realm.setTile(Layer::Submerged, {row, column}, 0, false);
	}
}
