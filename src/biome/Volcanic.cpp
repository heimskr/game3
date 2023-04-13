#include "Tileset.h"
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

	void Volcanic::generate(Index row, Index column, std::default_random_engine &, const noise::module::Perlin &perlin, const WorldGenParams &params) {
		Realm &realm = *getRealm();

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
			realm.setLayer1({row, column}, "base:tile/volcanic_sand"_id);
		} else if (0.85 < noise) {
			realm.setLayer1({row, column}, "base:tile/lava"_id);
		} else {
			realm.setLayer1({row, column}, "base:tile/volcanic_rock"_id);
		}
	}

	void Volcanic::postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &) {
		Realm &realm = *getRealm();
		static std::uniform_int_distribution distribution(0, 199);

		if (realm.getLayer1(row, column) == realm.getTileset()["base:tile/volcanic_sand"]) {
			if (distribution(rng) < 1) {
				Game &game = realm.getGame();
				std::vector<ItemStack> mushrooms {
					{game, "base:item/indigo_milkcap"_id},
					{game, "base:item/black_trumpet"_id},
					{game, "base:item/grey_knight"_id},
				};

				realm.add(TileEntity::create<ItemSpawner>(game, Position(row, column), 0.0002f, std::move(mushrooms)));
			}
		}
	}
}
