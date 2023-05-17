#include "Tileset.h"
#include "biome/Snowy.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/Tree.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	void Snowy::init(Realm &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
		forestPerlin = std::make_shared<noise::module::Perlin>();
		forestPerlin->SetSeed(-noise_seed * 3);
	}

	double Snowy::generate(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		const auto wetness    = params.wetness;
		const auto stoneLevel = params.stoneLevel;

		const double noise = perlin.GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.666);

		static const Identifier deeper_water  = "base:tile/deeper_water"_id;
		static const Identifier deep_water    = "base:tile/deep_water"_id;
		static const Identifier water         = "base:tile/water"_id;
		static const Identifier shallow_water = "base:tile/shallow_water"_id;
		static const Identifier sand          = "base:tile/sand"_id;
		static const Identifier dark_ice      = "base:tile/dark_ice"_id;
		static const Identifier light_ice     = "base:tile/light_ice"_id;
		static const Identifier snow          = "base:tile/snow"_id;
		static const Identifier stone         = "base:tile/stone"_id;

		if (noise < wetness) {
			realm.setTile(1, {row, column}, deeper_water, false, true);
		} else if (noise < wetness + 0.1) {
			realm.setTile(1, {row, column}, deep_water, false, true);
		} else if (noise < wetness + 0.2) {
			realm.setTile(1, {row, column}, water, false, true);
		} else if (noise < wetness + 0.3) {
			realm.setTile(1, {row, column}, shallow_water, false, true);
		} else if (noise < wetness + 0.39) {
			realm.setTile(1, {row, column}, sand, false, true);
		} else if (noise < wetness + 0.42) {
			realm.setTile(1, {row, column}, dark_ice, false, true);
		} else if (noise < wetness + 0.5) {
			realm.setTile(1, {row, column}, light_ice, false, true);
		} else if (stoneLevel < noise) {
			realm.setTile(1, {row, column}, stone, false, true);
		} else {
			realm.setTile(1, {row, column}, snow, false, true);
			const double forest_noise = forestPerlin->GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.5);
			if (params.forestThreshold < forest_noise) {
				uint8_t mod = abs(column) % 2;
				std::default_random_engine tree_rng(static_cast<uint_fast32_t>(forest_noise * 1'000'000'000.));
				if (std::uniform_int_distribution(0, 99)(tree_rng) < 50)
					mod = 1 - mod;
				if ((abs(row) % 2) == mod) {
					static const std::vector<Identifier> trees {"base:tile/winter_tree1"_id, "base:tile/winter_tree2"_id, "base:tile/winter_tree3"_id};
					realm.add(TileEntity::create<Tree>(realm.getGame(), choose(trees, rng), "base:tile/winter_stump"_id, Position(row, column), Tree::MATURITY));
				}
			}
		}

		return noise;
	}

	void Snowy::postgen(Index row, Index column, std::default_random_engine &, const noise::module::Perlin &perlin, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		constexpr double factor = 10;

		if (params.antiforestThreshold > perlin.GetValue(row / Biome::NOISE_ZOOM * factor, column / Biome::NOISE_ZOOM * factor, 0.)) {
			if (auto tile = realm.tileEntityAt({row, column}); tile && tile->is("base:te/tree"_id) && !std::dynamic_pointer_cast<Tree>(tile)->hasHive()) {
				realm.removeSafe(tile);
			}
		}
	}
}
