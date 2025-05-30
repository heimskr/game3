#include "util/Log.h"
#include "graphics/Tileset.h"
#include "game/Game.h"
#include "realm/Cave.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "util/Cast.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/CaveGen.h"

namespace Game3::WorldGen {
	constexpr static double noise_zoom = 20.;

	void generateCave(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed, const ChunkRange &range) {
		assert(std::dynamic_pointer_cast<Cave>(realm));
		auto guard = realm->guardGeneration();
		realm->markGenerated(range);
		DefaultNoiseGenerator noisegen(noise_seed);

		const Index row_min = CHUNK_SIZE * range.topLeft.y;
		const Index row_max = CHUNK_SIZE * (range.bottomRight.y + 1) - 1;
		const Index column_min = CHUNK_SIZE * range.topLeft.x;
		const Index column_max = CHUNK_SIZE * (range.bottomRight.x + 1) - 1;

		std::vector<uint8_t> was_object;
		was_object.reserve((row_max - row_min + 1) * (column_max - column_min + 1));

		for (Index row = row_min; row <= row_max; ++row) {
			for (Index column = column_min; column <= column_max; ++column) {
				was_object.push_back(generateCaveTile(realm, row, column, noisegen, rng)? 0 : 1);
			}
		}

		for (Index row = row_max; row >= row_min; --row) {
			for (Index column = column_max; column >= column_min; --column) {
				if (was_object.back()) {
					realm->autotile(Position{row, column}, Layer::Objects);
				} else {
					realm->autotile(Position{row, column}, Layer::Bedrock);
					realm->autotile(Position{row, column}, Layer::Soil);
				}
				was_object.pop_back();
			}
		}
	}

	namespace {
		TileID selectRareOre(const std::shared_ptr<Realm> &realm, Index row, Index column, std::default_random_engine &rng) {
			std::unique_lock<DefaultMutex> lock;
			auto &voronoi = safeDynamicCast<Cave>(realm)->getOreVoronoi(Position{row, column}.getChunk(), lock, rng);
			return voronoi[TileProvider::remainder(column), TileProvider::remainder(row)];
		}
	}

	bool generateNormalCaveTile(const std::shared_ptr<Realm> &realm, Index row, Index column, const NoiseGenerator &noisegen, std::default_random_engine &rng) {
		const double noise = noisegen(row / noise_zoom, column / noise_zoom, 0.1);

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
		} else if (noise < -.15) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall", false);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void", false);
		} else if (noise < -.1) {
			realm->setTile(Layer::Objects, {row, column}, selectRareOre(realm, row, column, rng), false);
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
		} else if (noise < .135) {
			realm->setTile(Layer::Objects, {row, column}, selectRareOre(realm, row, column, rng), false);
		} else {
			// TODO: move to mushroom caves
			constexpr static double extra_zoom = 5.;
			const double brine_noise = std::abs(noisegen(row / (extra_zoom * noise_zoom), column / (extra_zoom * noise_zoom), 541713.));
			realm->setTile(Layer::Bedrock, {row, column}, "base:tile/stone", false);

			if (0.666 < noise && brine_noise < 0.05) {
				if (brine_noise < 0.03) {
					realm->setFluid({row, column}, "base:fluid/brine", FluidTile::FULL, true);
				}
			} else {
				realm->setTile(Layer::Soil, {row, column}, "base:tile/cave_dirt", false);
			}

			return true;
		}

		return false;
	}

	bool generateGrimCaveTile(const std::shared_ptr<Realm> &realm, Index row, Index column, const NoiseGenerator &noisegen) {
		const double noise = noisegen(row / noise_zoom, column / noise_zoom, 0.1);

		Position position{row, column};

		if (noise < -.95) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.85) {
		// 	realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
		// 	realm->setTile(Layer::Highest, position, "base:tile/void", false);
		// } else if (noise < -.825) {
			realm->setTile(Layer::Objects, position, "base:tile/grim_diamond", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.725) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.7) {
			realm->setTile(Layer::Objects, position, "base:tile/grim_fireopal", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.6) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.55) {
			realm->setTile(Layer::Objects, position, "base:tile/grim_uranium", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.45) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.375) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < -.1) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
			realm->setTile(Layer::Highest, position, "base:tile/void", false);
		} else if (noise < .1) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
		} else if (noise < .11) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
		} else if (noise < .1125) {
			realm->setTile(Layer::Objects, position, "base:tile/grim_diamond", false);
		} else if (noise < .12) {
			realm->setTile(Layer::Objects, position, "base:tile/grim_uranium", false);
		} else if (noise < .1225) {
			realm->setTile(Layer::Objects, position, "base:tile/grim_fireopal", false);
		} else if (noise < .13) {
			realm->setTile(Layer::Objects, position, "base:tile/grimstone", false);
		} else {
			realm->setTile(Layer::Soil, position, "base:tile/grimdirt", false);

			constexpr static double extra_zoom = 6.66;
			const double lava_noise = std::abs(noisegen(row / (extra_zoom * noise_zoom), column / (extra_zoom * noise_zoom), 1474.));
			if (lava_noise < 0.0666) {
				realm->setFluid(position, "base:fluid/lava", FluidTile::FULL, true);
			}

			return true;
		}

		return false;
	}

	bool generateCaveTile(const std::shared_ptr<Realm> &realm, Index row, Index column, const NoiseGenerator &noisegen, std::default_random_engine &rng) {
		constexpr static double biome_zoom = noise_zoom * 10.;
		const double biome_noise = noisegen(row / biome_zoom, column / biome_zoom, 5.0);

		if (biome_noise < -0.5) {
			return generateGrimCaveTile(realm, row, column, noisegen);
		}

		return generateNormalCaveTile(realm, row, column, noisegen, rng);
	}

	void generateCaveFull(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed, const Position &exit_position, Position &entrance, RealmID parent_realm, const ChunkRange &range) {
		Timer timer{"CaveGenFull"};
		auto guard = realm->guardGeneration();
		realm->markGenerated(range);
		DefaultNoiseGenerator noisegen(noise_seed);

		const Index row_min = CHUNK_SIZE * range.topLeft.y;
		const Index row_max = CHUNK_SIZE * (range.bottomRight.y + 1) - 1;
		const Index column_min = CHUNK_SIZE * range.topLeft.x;
		const Index column_max = CHUNK_SIZE * (range.bottomRight.x + 1) - 1;

		range.iterate([&](ChunkPosition chunk_position) {
			realm->tileProvider.ensureAllChunks(chunk_position);
			realm->tileProvider.updateChunk(chunk_position);
		});

		std::vector<uint8_t> was_object;
		was_object.reserve((row_max - row_min + 1) * (column_max - column_min + 1));

		for (Index row = row_min; row <= row_max; ++row) {
			for (Index column = column_min; column <= column_max; ++column) {
				was_object.push_back(generateCaveTile(realm, row, column, noisegen, rng)? 0 : 1);
			}
		}

		for (Index row = row_max; row >= row_min; --row) {
			for (Index column = column_max; column >= column_min; --column) {
				if (was_object.back()) {
					realm->autotile(Position{row, column}, Layer::Objects);
				} else {
					realm->autotile(Position{row, column}, Layer::Bedrock);
					realm->autotile(Position{row, column}, Layer::Soil);
				}
				was_object.pop_back();
			}
		}

		entrance = exit_position / Cave::SCALE - Position(1, 0);

		for (const Position &position : {entrance, entrance + Position(1, 0)}) {
			realm->setTile(Layer::Objects, position, 0);
			realm->setTile(Layer::Highest, position, 0);
			if (realm->tryTile(Layer::Bedrock, position) == 0) {
				realm->setTile(Layer::Bedrock, position, "base:tile/stone");
			}
			if (realm->tryTile(Layer::Soil, position) == 0) {
				realm->setTile(Layer::Soil, position, "base:tile/cave_dirt");
			}
		}

		TileEntity::spawn<Building>(realm, "base:tile/ladder", entrance, parent_realm, exit_position);
		timer.stop();
		Timer::summary();
		Timer::clear();
	}
}
