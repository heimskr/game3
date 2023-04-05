#include "Tiles.h"
#include "biome/Volcanic.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"
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

	void Volcanic::postgen(Index row, Index column, std::default_random_engine &, const noise::module::Perlin &) {
		(void) row; (void) column;
	}
}
