#include <iostream>

#include "Log.h"
#include "graphics/Tileset.h"
#include "game/Game.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/CaveGen.h"

namespace Game3::WorldGen {
	constexpr static double noise_zoom = 20.;

	void generateCave(const std::shared_ptr<Realm> &realm, std::default_random_engine &, int noise_seed, const ChunkRange &range) {
		auto guard = realm->guardGeneration();
		realm->markGenerated(range);
		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

		const Index row_min = CHUNK_SIZE * range.topLeft.y;
		const Index row_max = CHUNK_SIZE * (range.bottomRight.y + 1) - 1;
		const Index column_min = CHUNK_SIZE * range.topLeft.x;
		const Index column_max = CHUNK_SIZE * (range.bottomRight.x + 1) - 1;

		std::vector<Layer> layers;
		layers.reserve((row_max - row_min + 1) * (column_max - column_min + 1));

		for (Index row = row_min; row <= row_max; ++row)
			for (Index column = column_min; column <= column_max; ++column)
				layers.push_back(generateCaveTile(realm, row, column, perlin)? Layer::Terrain : Layer::Objects);

		for (Index row = row_max; row >= row_min; --row) {
			for (Index column = column_max; column >= column_min; --column) {
				realm->autotile(Position{row, column}, layers.back());
				layers.pop_back();
			}
		}
	}

	bool generateNormalCaveTile(const std::shared_ptr<Realm> &realm, Index row, Index column, const noise::module::Perlin &perlin) {
		const double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.1);

		if (noise < -.95) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_iron", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.85) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.825) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_diamond", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.725) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.7) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_gold", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.6) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.55) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_copper", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.45) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.375) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_coal", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.1) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < .1) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall", false);
		} else if (noise < .11) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_iron", false);
		} else if (noise < .1125) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_diamond", false);
		} else if (noise < .12) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_copper", false);
		} else if (noise < .1225) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_gold", false);
		} else if (noise < .13) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_coal", false);
		} else {

			// TODO: move to mushroom caves
			constexpr static double extra_zoom = 5.;
			const double brine_noise = std::abs(perlin.GetValue(row / (extra_zoom * noise_zoom), column / (extra_zoom * noise_zoom), 541713.));
			if (0.666 < noise && brine_noise < 0.05) {
				realm->setTile(Layer::Terrain, {row, column}, "base:tile/stone", false);
				if (brine_noise < 0.03)
					realm->setFluid({row, column}, "base:fluid/brine", FluidTile::FULL, true);
			} else
				realm->setTile(Layer::Terrain, {row, column}, "base:tile/cave_dirt", false);

			return true;
		}

		return false;
	}

	bool generateGrimCaveTile(const std::shared_ptr<Realm> &realm, Index row, Index column, const noise::module::Perlin &perlin) {
		const double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.1);

		if (noise < -.95) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.85) {
		// 	realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
		// 	realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		// } else if (noise < -.825) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grim_diamond", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.725) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.7) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grim_fireopal", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.6) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.55) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grim_uranium", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.45) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.375) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.1) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < .1) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
		} else if (noise < .11) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
		} else if (noise < .1125) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grim_diamond", false);
		} else if (noise < .12) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grim_uranium", false);
		} else if (noise < .1225) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grim_fireopal", false);
		} else if (noise < .13) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/grimstone", false);
		} else {
			realm->setTile(Layer::Terrain, {row, column}, "base:tile/grimdirt", false);

			constexpr static double extra_zoom = 6.66;
			const double lava_noise = std::abs(perlin.GetValue(row / (extra_zoom * noise_zoom), column / (extra_zoom * noise_zoom), 1474.));
			if (lava_noise < 0.0666)
				realm->setFluid({row, column}, "base:fluid/lava", lava_noise < 0.04? FluidTile::FULL : FluidTile::FULL * std::cos(9001. * (lava_noise - 0.04)), true);

			return true;
		}

		return false;
	}

	bool generateCaveTile(const std::shared_ptr<Realm> &realm, Index row, Index column, const noise::module::Perlin &perlin) {
		constexpr static double biome_zoom = noise_zoom * 10.;
		const double biome_noise = perlin.GetValue(row / biome_zoom, column / biome_zoom, 5.0);

		if (biome_noise < -0.5)
			return generateGrimCaveTile(realm, row, column, perlin);

		return generateNormalCaveTile(realm, row, column, perlin);
	}

	void generateCaveFull(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed, const Position &exit_position, Position &entrance, RealmID parent_realm, const ChunkRange &range) {
		Timer timer{"CaveGenFull"};
		auto guard = realm->guardGeneration();
		realm->markGenerated(range);
		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

		const Index row_min = CHUNK_SIZE * range.topLeft.y;
		const Index row_max = CHUNK_SIZE * (range.bottomRight.y + 1) - 1;
		const Index column_min = CHUNK_SIZE * range.topLeft.x;
		const Index column_max = CHUNK_SIZE * (range.bottomRight.x + 1) - 1;

		std::vector<Position> inside;

		range.iterate([&](ChunkPosition chunk_position) {
			realm->tileProvider.ensureAllChunks(chunk_position);
			realm->tileProvider.updateChunk(chunk_position);
		});

		std::vector<Layer> layers;
		layers.reserve((row_max - row_min + 1) * (column_max - column_min + 1));

		for (Index row = row_min; row <= row_max; ++row) {
			for (Index column = column_min; column <= column_max; ++column) {
				if (generateCaveTile(realm, row, column, perlin)) {
					inside.emplace_back(row, column);
					layers.push_back(Layer::Terrain);
				} else
					layers.push_back(Layer::Objects);
			}
		}

		for (Index row = row_max; row >= row_min; --row) {
			for (Index column = column_max; column >= column_min; --column) {
				realm->autotile(Position{row, column}, layers.back());
				layers.pop_back();
			}
		}

		if (inside.empty())
			entrance = {0, 0};
		else
			entrance = choose(inside, rng);

		TileEntity::spawn<Building>(realm, "base:tile/ladder", entrance, parent_realm, exit_position);
		timer.stop();
		Timer::summary();
		Timer::clear();
	}
}
