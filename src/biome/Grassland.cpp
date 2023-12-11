#include <unordered_set>

#include "graphics/Tileset.h"
#include "biome/Grassland.h"
#include "entity/Chicken.h"
#include "entity/Cyclops.h"
#include "entity/Dog.h"
#include "entity/Pig.h"
#include "entity/Sheep.h"
#include "game/Game.h"
#include "item/Item.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/ItemSpawner.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	namespace {
		const std::vector<Identifier> grasses{
			"base:tile/grass_alt1", "base:tile/grass_alt2",
			"base:tile/grass", "base:tile/grass", "base:tile/grass", "base:tile/grass",
		};

		const std::unordered_set<Identifier> trees{
			"base:tile/tree1",
			"base:tile/tree2",
			"base:tile/tree3",
			"base:tile/tree1_empty",
			"base:tile/tree2_empty",
			"base:tile/tree3_empty"
		};

		const std::unordered_set<Identifier> grassSet{grasses.begin(), grasses.end()};

		/** Tries to generate a lilypad starting with the given location as the top left corner. */
		bool generateLilypad(const Place &place, bool flowerful) {
			Realm &realm = *place.realm;
			Tileset &tileset = realm.getTileset();

			constexpr static Layer layer = Layer::Objects;
			const TileID empty = tileset.getEmptyID();
			const FluidID water = safeCast<FluidID>(realm.getGame().registry<FluidRegistry>().at("base:fluid/water")->registryID);

			for (Index y_offset = 0; y_offset <= 1; ++y_offset) {
				for (Index x_offset = 0; x_offset <= 1; ++x_offset) {
					Position position = place.position + Position{y_offset, x_offset};
					if (auto fluid = realm.tryFluid(position); !fluid || fluid->id != water)
						return false;
					if (auto tile = realm.tryTile(layer, position); !tile || tile != empty)
						return false;
				}
			}

			realm.setTile(layer, place.position, flowerful? "base:tile/lilypad_flowerful_nw" : "base:tile/lilypad_flowerless_nw");
			realm.setTile(layer, place.position + Position{0, 1}, flowerful? "base:tile/lilypad_flowerful_ne" : "base:tile/lilypad_flowerless_ne");
			realm.setTile(layer, place.position + Position{1, 0}, flowerful? "base:tile/lilypad_flowerful_sw" : "base:tile/lilypad_flowerless_sw");
			realm.setTile(layer, place.position + Position{1, 1}, "base:tile/lilypad_se");
			return true;
		}
	}

	void Grassland::init(Realm &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
		forestPerlin = std::make_shared<noise::module::Perlin>();
		forestPerlin->SetSeed(-noise_seed * 3);
	}

	double Grassland::generate(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		const auto wetness    = params.wetness;
		const auto stoneLevel = params.stoneLevel;
		auto &tileset = realm.getTileset();
		const double noise = perlin.GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.666);

		static const Identifier deeper_water  = "base:tile/deeper_water";
		static const Identifier deep_water    = "base:tile/deep_water";
		static const Identifier water         = "base:tile/water";
		static const Identifier shallow_water = "base:tile/shallow_water";
		static const Identifier sand          = "base:tile/sand";
		static const Identifier light_grass   = "base:tile/light_grass";
		static const Identifier stone         = "base:tile/stone";
		static const Identifier forest_floor  = "base:tile/forest_floor";
		static const Identifier water_fluid   = "base:fluid/water";

		if (noise < wetness + 0.3) {
			realm.setTile(Layer::Terrain, {row, column}, sand, false);
			realm.setFluid({row, column}, water_fluid, params.getFluidLevel(noise, 0.3), true);
		} else if (noise < wetness + 0.4) {
			realm.setTile(Layer::Terrain, {row, column}, sand, false);
		} else if (noise < wetness + 0.5) {
			realm.setTile(Layer::Terrain, {row, column}, light_grass, false);
		} else if (stoneLevel < noise) {
			realm.setTile(Layer::Terrain, {row, column}, stone, false);
		} else {
			if (std::uniform_int_distribution(0, 15)(rng) == 0)
				realm.setTile(Layer::Terrain, {row, column}, choose(tileset.getTilesByCategory("base:category/small_flowers"), rng), false);
			else
				realm.setTile(Layer::Terrain, {row, column}, choose(grasses, rng), false);
			const double forest_noise = forestPerlin->GetValue(row / Biome::NOISE_ZOOM, column / Biome::NOISE_ZOOM, 0.5);
			if (params.forestThreshold < forest_noise) {
				std::default_random_engine tree_rng(static_cast<uint_fast32_t>(forest_noise * 1'000'000'000.));
				if ((abs(row) % 2) == (std::uniform_int_distribution(0, 39)(tree_rng) < 20))
					realm.setTile(Layer::Submerged, {row, column}, choose(trees, rng), false);
				realm.setTile(Layer::Terrain, {row, column}, forest_floor, false);
			}
		}

		return noise;
	}

	void Grassland::postgen(Index row, Index column, std::default_random_engine &rng, const noise::module::Perlin &perlin, const WorldGenParams &params) {
		Realm &realm = *getRealm();
		constexpr double factor = 10;

		if (params.antiforestThreshold > perlin.GetValue(row / Biome::NOISE_ZOOM * factor, column / Biome::NOISE_ZOOM * factor, 0.))
			if (auto tile = realm.tryTile(Layer::Submerged, {row, column}); tile && trees.contains(realm.getTileset()[*tile]))
				realm.setTile(Layer::Submerged, {row, column}, 0, false);

		const auto &tileset = realm.getTileset();
		const auto tile1 = tileset[realm.getTile(Layer::Terrain, {row, column})];

		if (water == FluidID(-1))
			water = safeCast<FluidID>(realm.getGame().registry<FluidRegistry>().at("base:fluid/water")->registryID);

		if (const auto fluid = realm.tryFluid({row, column}); fluid && fluid->id == water) {
			const double probability = 0.01 * std::pow(std::cos(std::min(1.6, 8.0 * (double(fluid->level) / FluidTile::FULL - 0.7))), 5.);
			if (std::uniform_real_distribution(0.0, 1.0)(rng) <= probability) {
				const int reeds_rand = std::uniform_int_distribution(1, 2)(rng);
				realm.setTile(Layer::Objects, {row, column}, reeds_rand == 1? "base:tile/reeds_1" : "base:tile/reeds_2");
			}
		}

		if (const int lilypad_rand = std::uniform_int_distribution(1, 2000)(rng); lilypad_rand <= 4)
			generateLilypad(Place({row, column}, realm.shared_from_this()), lilypad_rand <= 2);

		if (grassSet.contains(tile1) && realm.middleEmpty({row, column})) {
			if (std::uniform_int_distribution(1, 100)(rng) <= 2)
				realm.setTile(Layer::Submerged, {row, column}, choose(tileset.getCategoryIDs("base:category/flowers"), rng), false);

			std::shared_ptr<LivingEntity> spawned_entity;

			switch (std::uniform_int_distribution(1, 600)(rng)) {
				case 1:
				case 2:
					spawned_entity = realm.spawn<Sheep>({row, column});
					break;
				case 3:
				case 4:
					spawned_entity = realm.spawn<Pig>({row, column});
					break;
				case 5:
				case 6:
					spawned_entity = realm.spawn<Chicken>({row, column});
					break;
				case 7:
					spawned_entity = realm.spawn<Dog>({row, column});
					break;
			}

			if (spawned_entity)
				spawned_entity->direction = randomDirection();
		}
	}
}
