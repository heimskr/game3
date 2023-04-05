#include "Tiles.h"
#include "biome/Grassland.h"
#include "lib/noise.h"
#include "realm/Realm.h"
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

		static const std::vector<TileID> grasses {
			Monomap::GRASS_ALT1, Monomap::GRASS_ALT2,
			Monomap::GRASS, Monomap::GRASS, Monomap::GRASS, Monomap::GRASS, Monomap::GRASS, Monomap::GRASS, Monomap::GRASS
		};

		const double noise = perlin.GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.666);
		savedNoise[row * realm.getWidth() + column] = noise;

		if (noise < THRESHOLD) {
			realm.setLayer1(row, column, Monomap::DEEPER_WATER);
		} else if (noise < THRESHOLD + 0.1) {
			realm.setLayer1(row, column, Monomap::DEEP_WATER);
		} else if (noise < THRESHOLD + 0.2) {
			realm.setLayer1(row, column, Monomap::WATER);
		} else if (noise < THRESHOLD + 0.3) {
			realm.setLayer1(row, column, Monomap::SHALLOW_WATER);
		} else if (noise < THRESHOLD + 0.4) {
			realm.setLayer1(row, column, Monomap::SAND);
		} else if (noise < THRESHOLD + 0.5) {
			realm.setLayer1(row, column, Monomap::LIGHT_GRASS);
		} else if (0.8 < noise) {
			realm.setLayer1(row, column, Monomap::STONE);
		} else {
			realm.setLayer1(row, column, choose(grasses, rng));
			const double forest_noise = forestPerlin->GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.5);
			if (0.5 < forest_noise) {
				realm.add(TileEntity::create<Tree>(rng, Monomap::TREE1 + rand() % 3, Monomap::TREE0, Position(row, column), Tree::MATURITY));
				realm.setLayer1(row, column, Monomap::FOREST_FLOOR);
			}
		}
	}

	void Grassland::postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin) {
		Realm &realm = *getRealm();
		constexpr double factor = 10;
		static std::uniform_int_distribution distribution(1, 100);

		if (-0.4 > perlin.GetValue(row / Biome::NOISE_ZOOM * factor, column / Biome::NOISE_ZOOM * factor, 0.))
			if (auto tile = realm.tileEntityAt({row, column}); tile && tile->getID() == TileEntity::TREE && !std::dynamic_pointer_cast<Tree>(tile)->hasHive()) {
				realm.remove(tile);
				if (distribution(rng) < 5) {
					// TODO: add mushroom spawner
				}
			}
	}
}
