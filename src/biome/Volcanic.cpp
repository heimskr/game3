#include "graphics/Tileset.h"
#include "biome/Volcanic.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	void Volcanic::init(const std::shared_ptr<Realm> &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
	}

	double Volcanic::generate(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &params, double suggested_noise) {
		RealmPtr realm = getRealm();
		const auto wetness = params.wetness;

		static const Identifier volcanic_sand = "base:tile/volcanic_sand"_id;
		static const Identifier volcanic_rock = "base:tile/volcanic_rock"_id;
		static const Identifier water_fluid   = "base:fluid/water"_id;
		static const Identifier lava_fluid    = "base:fluid/lava"_id;

		Position position{row, column};

		realm->setTile(Layer::Bedrock, position, volcanic_rock, false);

		if (suggested_noise < wetness + 0.3) {
			realm->setTile(Layer::Soil, position, volcanic_sand, false);
			realm->setFluid(position, water_fluid, params.getFluidLevel(suggested_noise, 0.3), true);
		} else if (suggested_noise < wetness + 0.4) {
			realm->setTile(Layer::Soil, position, volcanic_sand, false);
		} else if (0.85 < suggested_noise) {
			realm->setFluid(position, lava_fluid, FluidTile::FULL, true);
		}

		return suggested_noise;
	}

	void Volcanic::postgen(Index row, Index column, std::default_random_engine &rng, const NoiseGenerator &, const WorldGenParams &) {
		RealmPtr realm = getRealm();
		std::uniform_int_distribution distribution{0, 199};

		if (realm->getTile(Layer::Soil, {row, column}) == realm->getTileset()["base:tile/volcanic_sand"_id]) {
			if (distribution(rng) < 1) {
				std::shared_ptr<Game> game = realm->getGame();
				static std::vector<Identifier> mushrooms{
					"base:item/indigo_milkcap",
					"base:item/black_trumpet",
					"base:item/grey_knight",
				};
			}
		}
	}
}
