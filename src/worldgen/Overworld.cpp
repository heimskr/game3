#include "threading/ThreadContext.h"
#include "graphics/Tileset.h"
#include "biome/Biome.h"
#include "biome/Grassland.h"
#include "game/Game.h"
#include "lib/noise.h"
#include "realm/Overworld.h"
#include "realm/Realm.h"
#include "threading/Waiter.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/Town.h"
#include "worldgen/VillageGen.h"
#include "worldgen/WorldGen.h"

#include <semaphore>
#include <thread>

// #define GENERATE_RIVERS

namespace Game3::WorldGen {
	void generateOverworld(const std::shared_ptr<Realm> &realm, size_t noise_seed, const WorldGenParams &params, const ChunkRange &range, bool initial_generation) {
		realm->markGenerated(range);
		Timer overworld_timer("GenOverworld");

		auto guard = realm->guardGeneration();

		const auto width  = range.tileWidth();
		const auto height = range.tileHeight();

		TileProvider &provider = realm->tileProvider;
		const Tileset &tileset = realm->getTileset();

		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y) {
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x) {
				provider.ensureAllChunks(ChunkPosition{x, y});
				for (const Layer layer: allLayers) {
					TileChunk &chunk = provider.getTileChunk(layer, ChunkPosition{x, y});
					auto lock = chunk.uniqueLock();
					chunk.assign(chunk.size(), tileset.getEmptyID());
				}
			}
		}

		const size_t regions_x = updiv(width,  CHUNK_SIZE);
		const size_t regions_y = updiv(height, CHUNK_SIZE);
		const size_t job_count = regions_x * regions_y;

		DefaultNoiseGenerator noisegen2(noise_seed * 3 - 1);

		auto biomes = Biome::getMap(realm, noise_seed);
		auto get_biome = [&](Index row, Index column) -> Biome & {
			if (auto biome_type = provider.copyBiomeType(Position(row, column)))
				return *biomes.at(*biome_type);
			throw std::runtime_error("Couldn't get biome type at (" + std::to_string(row) + ", " + std::to_string(column) + ')');
		};

		const Index range_row_min = range.rowMin();
		const Index range_row_max = range.rowMax();
		const Index range_column_min = range.columnMin();
		const Index range_column_max = range.columnMax();

		std::vector<float> biome_noise;

		{
			const float zoom = params.biomeZoom;
			noisegen2.fill(biome_noise, range_column_min, range_row_min, range_column_max - range_column_min + 1, range_row_max - range_row_min + 1, 1.f / zoom);
		}
		size_t biome_noise_index = 0;

		for (Index row = range_row_min; row <= range_row_max; ++row) {
			for (Index column = range_column_min; column <= range_column_max; ++column) {
				const double noise = std::min(1., std::max(-1., double(biome_noise[biome_noise_index++])));
				std::unique_lock<std::shared_mutex> lock;
				BiomeType &type = provider.findBiomeType(Position(row, column), &lock);

				if (noise < -0.9)
					type = Biome::VOLCANIC;
				else if (noise < -0.6)
					type = Biome::DESERT;
				else if (0.9 < noise)
					type = Biome::SNOWY;
				else
					type = Biome::GRASSLAND;
			}
		}

		DefaultNoiseGenerator noisegen(noise_seed);

#ifdef GENERATE_RIVERS
		DefaultNoiseGenerator river_noise(-5 * noise_seed + 1);
#endif

		GamePtr game_ptr = realm->getGame();

		pool.start();
		Waiter waiter(job_count);

		for (size_t thread_row = 0; thread_row < regions_y; ++thread_row) {
			const Index row_min = range_row_min + thread_row * CHUNK_SIZE;
			// Compare with <, not <=
			const Index row_max = row_min + CHUNK_SIZE;

			for (size_t thread_col = 0; thread_col < regions_x; ++thread_col) {
				const Index col_min = range_column_min + thread_col * CHUNK_SIZE;
				// Compare with <, not <=
				const Index col_max = col_min + CHUNK_SIZE;

				pool.add([&, game_ptr, row_min, row_max, col_min, col_max](ThreadPool &, size_t) {
					threadContext = {static_cast<uint_fast32_t>(noise_seed - 1'000'000ul * row_min + col_min), row_min, row_max, col_min, col_max};

					auto guard = realm->guardGeneration();

					std::vector<double> saved_noise((row_max - row_min) * (col_max - col_min));

					size_t noise_index = 0;

					// Timer noise_timer("BiomeGeneration");
					std::vector<float> suggested_noise;
					noisegen.fill(suggested_noise, col_min, row_min, col_max - col_min, row_max - row_min, 1.f / params.noiseZoom);

					for (auto row = row_min; row < row_max; ++row) {
						for (auto column = col_min; column < col_max; ++column) {
							auto &biome = get_biome(row, column);
							saved_noise[noise_index] = biome.generate(row, column, threadContext.rng, noisegen, params, suggested_noise[noise_index]);
							++noise_index;
#ifdef GENERATE_RIVERS
							constexpr double river_zoom = 400.;
							const auto river = river_noise(row / river_zoom, column / river_zoom, 0.5);
							constexpr double range = 0.05;
							constexpr double start = -range / 2;
							if (start <= river && river <= start + range) {
								realm->setFluid({row, column}, "base:fluid/water"_id, FluidTile::INFINITE);
							}
#endif
						}
					}
					// noise_timer.stop();

					// Timer resource_timer("Resources");
					std::vector<Position> resource_starts;
					resource_starts.reserve(width * height / 10);

					const auto ore_set = tileset.getCategoryIDs("base:category/orespawns"_id);

					for (auto row = row_min; row < row_max; ++row) {
						for (auto column = col_min; column < col_max; ++column) {
							if (ore_set.contains(realm->getTile(Layer::Bedrock, {row, column})) && !realm->hasFluid({row, column})) {
								resource_starts.push_back({row, column});
							}
						}
					}

					std::shuffle(resource_starts.begin(), resource_starts.end(), threadContext.rng);
					GamePtr game = realm->getGame();
					auto &ores = game->registry<OreRegistry>();

					auto add_resources = [&](double threshold, const Identifier &ore_name) {
						auto ore = ores.at(ore_name);
						for (size_t i = 0, max = resource_starts.size() / 1000; i < max; ++i) {
							const Position &position = resource_starts.back();
							const Index index = (position.row - row_min) * CHUNK_SIZE + (position.column - col_min);
							if (Grassland::THRESHOLD + threshold <= saved_noise[index]) {
								TileEntity::spawn<OreDeposit>(realm, *ore, position);
							}
							resource_starts.pop_back();
						}
					};

					add_resources(1.0, "base:ore/iron");
					add_resources(0.5, "base:ore/copper");
					add_resources(0.5, "base:ore/gold");
					add_resources(0.5, "base:ore/diamond");
					add_resources(0.5, "base:ore/coal");
					// TODO: oil
					// resource_timer.stop();

					--waiter;
				});
			}
		}

		waiter.wait();

		range.iterate([&](ChunkPosition chunk_position) {
			tryGenerateVillage(realm, chunk_position, pool);
		});

		Timer postgen_timer("Postgen");

		waiter.reset(regions_y * regions_x);

		for (size_t thread_row = 0; thread_row < regions_y; ++thread_row) {
			const Index row_min = range_row_min + thread_row * CHUNK_SIZE;
			// Compare with <, not <=
			const Index row_max = row_min + CHUNK_SIZE;
			for (size_t thread_col = 0; thread_col < regions_x; ++thread_col) {
				const Index col_min = range_column_min + thread_col * CHUNK_SIZE;
				// Compare with <, not <=
				const Index col_max = col_min + CHUNK_SIZE;
				pool.add([realm, &waiter, &get_biome, &noisegen, &params, noise_seed, row_min, row_max, col_min, col_max](ThreadPool &, size_t) {
					threadContext = {uint_fast32_t(noise_seed - 1'000'000ul * row_min + col_min), row_min, row_max, col_min, col_max};
					for (Index row = row_min; row < row_max; ++row) {
						for (Index column = col_min; column < col_max; ++column) {
							realm->autotile({row, column}, Layer::Bedrock);
							realm->autotile({row, column}, Layer::Soil);
							get_biome(row, column).postgen(row, column, threadContext.rng, noisegen, params);
						}
					}
					--waiter;
				});
			}
		}

		waiter.wait();

		postgen_timer.stop();

		range.iterate([&](ChunkPosition chunk_position) {
			provider.updateChunk(chunk_position);
		});

		if (initial_generation)
			std::dynamic_pointer_cast<Overworld>(realm)->worldgenParams = params;

		{
			Timer pathmap_timer("RemakePathMap");
			realm->remakePathMap(range);
		}

		overworld_timer.stop();
		if (initial_generation)
			Timer::summary();
		Timer::clear();
	}
}
