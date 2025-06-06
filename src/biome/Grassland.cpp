#include "Options.h"
#include "biome/Grassland.h"
#include "entity/Chicken.h"
#include "entity/Crab.h"
#include "entity/Cyclops.h"
#include "entity/Dog.h"
#include "entity/Pig.h"
#include "entity/Sheep.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
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
			RealmPtr realm = place.realm;
			Tileset &tileset = realm->getTileset();

			constexpr static Layer layer = Layer::Objects;
			const TileID empty = tileset.getEmptyID();
			const FluidID water = safeCast<FluidID>(realm->getGame()->registry<FluidRegistry>().at("base:fluid/water")->registryID);

			for (Index y_offset = 0; y_offset <= 1; ++y_offset) {
				for (Index x_offset = 0; x_offset <= 1; ++x_offset) {
					Position position = place.position + Position{y_offset, x_offset};
					if (auto fluid = realm->tryFluid(position); !fluid || fluid->id != water) {
						return false;
					}
					if (auto tile = realm->tryTile(layer, position); !tile || tile != empty) {
						return false;
					}
				}
			}

			realm->setTile(layer, place.position, flowerful? "base:tile/lilypad_flowerful_nw" : "base:tile/lilypad_flowerless_nw");
			realm->setTile(layer, place.position + Position{0, 1}, flowerful? "base:tile/lilypad_flowerful_ne" : "base:tile/lilypad_flowerless_ne");
			realm->setTile(layer, place.position + Position{1, 0}, flowerful? "base:tile/lilypad_flowerful_sw" : "base:tile/lilypad_flowerless_sw");
			realm->setTile(layer, place.position + Position{1, 1}, "base:tile/lilypad_se");
			return true;
		}
	}

	void Grassland::init(const std::shared_ptr<Realm> &realm, int noise_seed) {
		Biome::init(realm, noise_seed);
		forestNoise.setSeed(-noise_seed * 3);
	}

	double Grassland::generate(Index row, Index column, std::default_random_engine &rng, const NoiseGenerator &, const WorldGenParams &params, double suggested_noise) {
		Realm &realm = *getRealm();
		const auto wetness    = params.wetness;
		const auto stoneLevel = params.stoneLevel;
		Tileset &tileset = realm.getTileset();

		static const Identifier sand         = "base:tile/sand";
		static const Identifier dirt         = "base:tile/dirt";
		static const Identifier stone        = "base:tile/stone";
		static const Identifier light_grass  = "base:tile/light_grass";
		static const Identifier water_fluid  = "base:fluid/water";
		static const Identifier forest_floor = "base:tile/forest_floor";

		Position position{row, column};

		realm.setTile(Layer::Bedrock, position, stone, false);

		if (suggested_noise < wetness + 0.3) {
			realm.setTile(Layer::Soil, position, sand, false);
			realm.setFluid(position, water_fluid, params.getFluidLevel(suggested_noise, 0.3), true);
		} else if (suggested_noise < wetness + 0.4) {
			realm.setTile(Layer::Soil, position, sand, false);
		} else if (suggested_noise < wetness + 0.5) {
			realm.setTile(Layer::Soil, position, sand, false);
			realm.setTile(Layer::Vegetation, position, light_grass, false);
		} else if (stoneLevel < suggested_noise) {
			// Do nothing; there's stone on the bedrock layer already.
		} else if (stoneLevel< suggested_noise + 0.1) {
			realm.setTile(Layer::Soil, position, dirt, false);
		} else {
			realm.setTile(Layer::Soil, position, dirt, false);
			realm.setTile(Layer::Vegetation, position, choose(grasses, rng), false);
			if (std::uniform_int_distribution{0, 15}(rng) == 0) {
				realm.setTile(Layer::Submerged, position, choose(tileset.getTilesByCategory("base:category/small_flowers"), rng), false);
			}

			const double forest_noise = forestNoise(row / params.noiseZoom, column / params.noiseZoom, 0.5);

			if (params.forestThreshold < forest_noise) {
				std::default_random_engine tree_rng(static_cast<uint_fast32_t>(forest_noise * 1'000'000'000.));
				if ((std::abs(row) % 2) == (std::uniform_int_distribution{0, 39}(tree_rng) < 20)) {
					realm.setTile(Layer::Submerged, position, choose(trees, rng), false);
				}
				realm.setTile(Layer::Vegetation, position, forest_floor, false);
			}
		}

		return suggested_noise;
	}

	void Grassland::postgen(Index row, Index column, std::default_random_engine &rng, const NoiseGenerator &noisegen, const WorldGenParams &params) {
		RealmPtr realm = getRealm();
		constexpr double factor = 10;
		const Tileset &tileset = realm->getTileset();
		const Position position{row, column};

		if (params.antiforestThreshold > noisegen(row / params.noiseZoom * factor, column / params.noiseZoom * factor, 0.)) {
			if (auto tile = realm->tryTile(Layer::Submerged, position); tile && trees.contains(tileset[*tile])) {
				realm->setTile(Layer::Submerged, position, 0, false);
			}
		}

		const Identifier soil_tile = tileset[realm->getTile(Layer::Soil, position)];

		if (water == FluidID(-1)) {
			water = safeCast<FluidID>(realm->getGame()->registry<FluidRegistry>().at("base:fluid/water")->registryID);
		}

		if (const auto fluid = realm->tryFluid(position); fluid && fluid->id == water) {
			const double probability = 0.01 * std::pow(std::cos(std::min(1.6, 8.0 * (double(fluid->level) / FluidTile::FULL - 0.7))), 5.);
			if (std::uniform_real_distribution(0.0, 1.0)(rng) <= probability) {
				const int reeds_rand = std::uniform_int_distribution{1, 2}(rng);
				realm->setTile(Layer::Objects, position, reeds_rand == 1? "base:tile/reeds_1" : "base:tile/reeds_2");
			}
		}

		if (const int lilypad_rand = std::uniform_int_distribution{1, 2000}(rng); lilypad_rand <= 4) {
			generateLilypad(Place(position, realm), lilypad_rand <= 2);
		}

		if (grassSet.contains(soil_tile)) {
			if (realm->middleEmpty(position)) {
				if constexpr (SPAWN_BIG_FLOWERS) {
					if (std::uniform_int_distribution{1, 100}(rng) <= 2) {
						realm->setTile(Layer::Submerged, position, choose(tileset.getCategoryIDs("base:category/flowers"), rng), false);
					}
				}

				std::shared_ptr<LivingEntity> spawned_entity;

				switch (std::uniform_int_distribution{1, 600}(rng)) {
					case 1:
					case 2:
						spawned_entity = realm->spawn<Sheep>(position);
						break;
					case 3:
					case 4:
						spawned_entity = realm->spawn<Pig>(position);
						break;
					case 5:
					case 6:
						spawned_entity = realm->spawn<Chicken>(position);
						break;
					case 7:
						spawned_entity = realm->spawn<Dog>(position);
						break;
					default:
						break;
				}

				if (spawned_entity) {
					spawned_entity->direction = randomDirection();
				}
			}
		} else if (soil_tile == "base:tile/sand") {
			if (realm->middleEmpty(position) && !realm->hasFluid(position) && std::uniform_int_distribution{1, 100}(rng) == 1) {
				std::shared_ptr<Crab> crab = realm->spawn<Crab>(position);
				crab->direction = randomDirection();
			}
		}
	}
}
