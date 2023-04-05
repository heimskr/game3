#include "Tiles.h"
#include "biome/Volcanic.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	void Volcanic::init(Realm &realm, int noise_seed, const std::shared_ptr<double[]> &saved_noise) {
		Biome::init(realm, noise_seed, saved_noise);
	}

	void Volcanic::generate(Index row, Index column, std::default_random_engine &, const noise::module::Perlin &perlin) {
		Realm &realm = *getRealm();

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
			realm.setLayer1(row, column, Monomap::VOLCANIC_SAND);
		} else if (0.85 < noise) {
			realm.setLayer1(row, column, Monomap::LAVA);
		} else {
			realm.setLayer1(row, column, Monomap::VOLCANIC_ROCK);
		}
	}

	void Volcanic::postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &) {
		Realm &realm = *getRealm();
		static std::uniform_int_distribution distribution(0, 199);

		if (realm.getLayer1(row, column) == Monomap::VOLCANIC_SAND) {
			if (distribution(rng) < 1) {
				static const std::vector<ItemStack> mushrooms {
					{Item::INDIGO_MILKCAP},
					{Item::BLACK_TRUMPET},
					{Item::GREY_KNIGHT},
				};

				realm.add(TileEntity::create<ItemSpawner>(Position(row, column), 0.0002f, mushrooms));
			}
		}
	}
}
