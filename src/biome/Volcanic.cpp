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
	void Volcanic::init(Realm &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
	}

	double Volcanic::generate(Index row, Index column, std::default_random_engine &, const NoiseGenerator &, const WorldGenParams &params, double suggested_noise) {
		Realm &realm = *getRealm();
		const auto wetness = params.wetness;

		static const Identifier volcanic_sand = "base:tile/volcanic_sand"_id;
		static const Identifier volcanic_rock = "base:tile/volcanic_rock"_id;
		static const Identifier water_fluid   = "base:fluid/water"_id;
		static const Identifier lava_fluid    = "base:fluid/lava"_id;

		if (suggested_noise < wetness + 0.3) {
			realm.setTile(Layer::Terrain, {row, column}, volcanic_sand, false);
			realm.setFluid({row, column}, water_fluid, params.getFluidLevel(suggested_noise, 0.3), true);
		} else if (suggested_noise < wetness + 0.4) {
			realm.setTile(Layer::Terrain, {row, column}, volcanic_sand, false);
		} else if (0.85 < suggested_noise) {
			realm.setTile(Layer::Terrain, {row, column}, volcanic_rock, false);
			realm.setFluid({row, column}, lava_fluid, FluidTile::FULL, true);
		} else {
			realm.setTile(Layer::Terrain, {row, column}, volcanic_rock, false);
		}

		return suggested_noise;
	}

	void Volcanic::postgen(Index row, Index column, std::default_random_engine &rng, const NoiseGenerator &, const WorldGenParams &) {
		Realm &realm = *getRealm();
		std::uniform_int_distribution distribution{0, 199};

		if (realm.getTile(Layer::Terrain, {row, column}) == realm.getTileset()["base:tile/volcanic_sand"_id]) {
			if (distribution(rng) < 1) {
				std::shared_ptr<Game> game = realm.getGame();
				std::vector<ItemStack> mushrooms {
					{game, "base:item/indigo_milkcap"_id},
					{game, "base:item/black_trumpet"_id},
					{game, "base:item/grey_knight"_id},
				};

				// TODO: use random ticks to spawn mushrooms
				// realm.add(TileEntity::create<ItemSpawner>(game, Position(row, column), 0.0002f, std::move(mushrooms)));
			}
		}
	}
}
