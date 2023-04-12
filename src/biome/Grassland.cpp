#include "Tileset.h"
#include "biome/Grassland.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/Tree.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	void Grassland::init(Realm &realm, int noise_seed, const std::shared_ptr<double[]> &saved_noise) {
		Biome::init(realm, noise_seed, saved_noise);
		forestPerlin = std::make_shared<noise::module::Perlin>();
		forestPerlin->SetSeed(-noise_seed * 3);
	}

	void Grassland::generate(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin) {
		Realm &realm = *getRealm();

		static const std::vector<Identifier> grasses {
			"base:tile/grass_alt1"_id, "base:tile/grass_alt2"_id,
			"base:tile/grass"_id, "base:tile/grass"_id, "base:tile/grass"_id, "base:tile/grass"_id,
		};

		const double noise = perlin.GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.666);
		savedNoise[row * realm.getWidth() + column] = noise;

		if (noise < THRESHOLD) {
			realm.setLayer1({row, column}, "base:tile/deeper_water"_id);
		} else if (noise < THRESHOLD + 0.1) {
			realm.setLayer1({row, column}, "base:tile/deep_water"_id);
		} else if (noise < THRESHOLD + 0.2) {
			realm.setLayer1({row, column}, "base:tile/water"_id);
		} else if (noise < THRESHOLD + 0.3) {
			realm.setLayer1({row, column}, "base:tile/shallow_water"_id);
		} else if (noise < THRESHOLD + 0.4) {
			realm.setLayer1({row, column}, "base:tile/sand"_id);
		} else if (noise < THRESHOLD + 0.5) {
			realm.setLayer1({row, column}, "base:tile/light_grass"_id);
		} else if (0.8 < noise) {
			realm.setLayer1({row, column}, "base:tile/stone"_id);
		} else {
			realm.setLayer1({row, column}, choose(grasses, rng));
			const double forest_noise = forestPerlin->GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.5);
			if (0.5 < forest_noise) {
				static const std::vector<Identifier> trees {"base:tile/tree1"_id, "base:tile/tree2"_id, "base:tile/tree3"_id};
				realm.add(TileEntity::create<Tree>(realm.getGame(), rng, choose(trees), "base:tile/tree0"_id, Position(row, column), Tree::MATURITY));
				realm.setLayer1({row, column}, "base:tile/forest_floor"_id);
			}
		}
	}

	void Grassland::postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin) {
		Realm &realm = *getRealm();
		constexpr double factor = 10;
		static std::uniform_int_distribution distribution(0, 99);

		if (-0.4 > perlin.GetValue(row / Biome::NOISE_ZOOM * factor, column / Biome::NOISE_ZOOM * factor, 0.)) {
			if (auto tile = realm.tileEntityAt({row, column}); tile && tile->is("base:te/tree"_id) && !std::dynamic_pointer_cast<Tree>(tile)->hasHive()) {
				Game &game = realm.getGame();
				realm.remove(tile);
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
