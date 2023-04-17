#include "Tileset.h"
#include "biome/Grassland.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/Tree.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	void Grassland::init(Realm &realm, int noise_seed, const std::shared_ptr<double[]> &saved_noise) {
		Biome::init(realm, noise_seed, saved_noise);
		forestPerlin = std::make_shared<noise::module::Perlin>();
		forestPerlin->SetSeed(-noise_seed * 3);
	}

	void Grassland::generate(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		const auto wetness    = params.wetness;
		const auto stoneLevel = params.stoneLevel;

		static const std::vector<Identifier> grasses {
			"base:tile/grass_alt1"_id, "base:tile/grass_alt2"_id,
			"base:tile/grass"_id, "base:tile/grass"_id, "base:tile/grass"_id, "base:tile/grass"_id,
		};

		auto &layer1  = realm.tilemap1->getTilesUnsafe();
		auto &tileset = *realm.tilemap1->tileset;
		const Index index = realm.getIndex(row, column);

		const double noise = perlin.GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.666);
		savedNoise[index] = noise;

		static const Identifier deeper_water  = "base:tile/deeper_water"_id;
		static const Identifier deep_water    = "base:tile/deep_water"_id;
		static const Identifier water         = "base:tile/water"_id;
		static const Identifier shallow_water = "base:tile/shallow_water"_id;
		static const Identifier sand          = "base:tile/sand"_id;
		static const Identifier light_grass   = "base:tile/light_grass"_id;
		static const Identifier stone         = "base:tile/stone"_id;
		static const Identifier forest_floor  = "base:tile/forest_floor"_id;

		if (noise < wetness) {
			layer1[index] = tileset[deeper_water];
		} else if (noise < wetness + 0.1) {
			layer1[index] = tileset[deep_water];
		} else if (noise < wetness + 0.2) {
			layer1[index] = tileset[water];
		} else if (noise < wetness + 0.3) {
			layer1[index] = tileset[shallow_water];
		} else if (noise < wetness + 0.4) {
			layer1[index] = tileset[sand];
		} else if (noise < wetness + 0.5) {
			layer1[index] = tileset[light_grass];
		} else if (stoneLevel < noise) {
			layer1[index] = tileset[stone];
		} else {
			layer1[index] = tileset[choose(grasses, rng)];
			const double forest_noise = forestPerlin->GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.5);
			if (params.forestThreshold < forest_noise) {
				uint8_t mod = column % 2;
				std::default_random_engine tree_rng(static_cast<uint_fast32_t>(forest_noise * 1'000'000'000.));
				if (std::uniform_int_distribution(0, 99)(tree_rng) < 10)
					mod = 1 - mod;
				if ((row % 2) == mod) {
					static const std::vector<Identifier> trees {"base:tile/tree1"_id, "base:tile/tree2"_id, "base:tile/tree3"_id};
					realm.add(TileEntity::create<Tree>(realm.getGame(), choose(trees, rng), "base:tile/tree0"_id, Position(row, column), Tree::MATURITY));
				}
				layer1[index] = tileset[forest_floor];
			}
		}
	}

	void Grassland::postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		constexpr double factor = 10;
		static std::uniform_int_distribution distribution(0, 99);

		if (params.antiforestThreshold > perlin.GetValue(row / Biome::NOISE_ZOOM * factor, column / Biome::NOISE_ZOOM * factor, 0.)) {
			if (auto tile = realm.tileEntityAt({row, column}); tile && tile->is("base:te/tree"_id) && !std::dynamic_pointer_cast<Tree>(tile)->hasHive()) {
				Game &game = realm.getGame();
				realm.removeSafe(tile);
				if (distribution(rng) < 3) {
					std::vector<ItemStack> mushrooms {
						{game, "base:item/saffron_milkcap"},
						{game, "base:item/saffron_milkcap"},
						{game, "base:item/saffron_milkcap"},
						{game, "base:item/honey_fungus"},
						{game, "base:item/honey_fungus"},
						{game, "base:item/brittlegill"},
					};

					realm.add(TileEntity::create<ItemSpawner>(game, Position(row, column), 0.00025f, std::move(mushrooms)));
				}
			}
		}
	}
}
