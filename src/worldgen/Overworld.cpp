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

		auto &provider = realm->tileProvider;

		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				provider.ensureAllChunks(ChunkPosition{x, y});

		const size_t regions_x = updiv(width,  CHUNK_SIZE);
		const size_t regions_y = updiv(height, CHUNK_SIZE);
		const size_t job_count = regions_x * regions_y;

		noise::module::Perlin p2;
		p2.SetSeed(noise_seed * 3 - 1);

		const auto &tileset = realm->getTileset();

		auto biomes = Biome::getMap(*realm, noise_seed);
		auto get_biome = [&](Index row, Index column) -> Biome & {
			if (auto biome_type = provider.copyBiomeType(Position(row, column)))
				return *biomes.at(*biome_type);
			throw std::runtime_error("Couldn't get biome type at (" + std::to_string(row) + ", " + std::to_string(column) + ')');
		};

		p2.SetNoiseQuality(noise::NoiseQuality::QUALITY_BEST);
		p2.SetFrequency(0.8);

		const Index range_row_min = range.rowMin();
		const Index range_row_max = range.rowMax();
		const Index range_column_min = range.columnMin();
		const Index range_column_max = range.columnMax();

		for (Index row = range_row_min; row <= range_row_max; ++row) {
			for (Index column = range_column_min; column <= range_column_max; ++column) {
				const double noise = std::min(1., std::max(-1., p2.GetValue(row / params.biomeZoom, column / params.biomeZoom, 0.0) * 5.));
				std::unique_lock<std::shared_mutex> lock;
				auto &type = provider.findBiomeType(Position(row, column), &lock);
				if (noise < -0.8)
					type = Biome::VOLCANIC;
				else if (noise < -0.5)
					type = Biome::DESERT;
				else if (0.7 < noise)
					type = Biome::SNOWY;
				else
					type = Biome::GRASSLAND;
			}
		}

		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

#ifdef GENERATE_RIVERS
		noise::module::Perlin river_perlin;
		river_perlin.SetSeed(-5 * noise_seed + 1);
#endif

		const GamePtr game_ptr = realm->getGame().shared_from_this();

		for (int32_t y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (int32_t x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				provider.ensureAllChunks(ChunkPosition{x, y});

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
					threadContext = {game_ptr, static_cast<uint_fast32_t>(noise_seed - 1'000'000ul * row_min + col_min), row_min, row_max, col_min, col_max};

					auto guard = realm->guardGeneration();

					std::vector<double> saved_noise((row_max - row_min) * (col_max - col_min));

					size_t noise_index = 0;

					// Timer noise_timer("BiomeGeneration");
					for (auto row = row_min; row < row_max; ++row) {
						for (auto column = col_min; column < col_max; ++column) {
							auto &biome = get_biome(row, column);
							saved_noise[noise_index++] = biome.generate(row, column, threadContext.rng, perlin, params);
#ifdef GENERATE_RIVERS
							constexpr double river_zoom = 400.;
							const auto river = river_perlin.GetValue(row / river_zoom, column / river_zoom, 0.5);
							constexpr double range = 0.05;
							constexpr double start = -range / 2;
							if (start <= river && river <= start + range) {
								realm->setFluid({row, column}, "base:fluid/water"_id, FluidTile::INFINITE, false, true);
							}
#endif
						}
					}
					// noise_timer.stop();

					// Timer resource_timer("Resources");
					std::vector<Position> resource_starts;
					resource_starts.reserve(width * height / 10);

					const auto ore_set = tileset.getCategoryIDs("base:category/orespawns"_id);

					for (auto row = row_min; row < row_max; ++row)
						for (auto column = col_min; column < col_max; ++column)
							if (ore_set.contains(realm->getTile(Layer::Terrain, {row, column})) && !realm->hasFluid({row, column}))
								resource_starts.push_back({row, column});

					std::shuffle(resource_starts.begin(), resource_starts.end(), threadContext.rng);
					Game &game = realm->getGame();
					auto &ores = game.registry<OreRegistry>();

					auto add_resources = [&](double threshold, const Identifier &ore_name) {
						auto ore = ores.at(ore_name);
						for (size_t i = 0, max = resource_starts.size() / 1000; i < max; ++i) {
							const Position &position = resource_starts.back();
							const Index index = (position.row - row_min) * CHUNK_SIZE + (position.column - col_min);
							if (Grassland::THRESHOLD + threshold <= saved_noise[index])
								realm->add(TileEntity::create<OreDeposit>(game, *ore, position));
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

		std::default_random_engine rng(noise_seed);

		if (initial_generation) {
			constexpr int m = 26, n = 34, pad = 2;
			Timer land_timer("GetLand");
			const auto starts = provider.getLand(*game_ptr, range, m + pad * 2, n + pad * 2);
			land_timer.stop();
			constexpr size_t chunk_size = 512;

			if (!starts.empty()) {
				Timer candidate_timer("Candidates");

				std::vector<Position> candidates;
				candidates.reserve(starts.size() / 16);
				const size_t chunk_max = updiv(starts.size(), chunk_size);

				std::mutex candidates_mutex;
				realm->randomLand = choose(starts, rng);

				waiter.reset(chunk_max);

				for (size_t chunk = 0; chunk < chunk_max; ++chunk) {
					pool.add([&, chunk](ThreadPool &, size_t) {
						std::vector<Position> thread_candidates;

						for (size_t i = chunk * chunk_size, max = std::min((chunk + 1) * chunk_size, starts.size()); i < max; ++i) {
							const auto position = starts[i];
							const Index row_start = position.row + pad;
							const Index row_end = row_start + m;
							const Index column_start = position.column + pad;
							const Index column_end = column_start + n;

							for (Index row = row_start; row < row_end; row += 2) {
								for (Index column = column_start; column < column_end; column += 2) {
									if (auto tile = provider.tryTile(Layer::Terrain, {row, column}); !tile || !tileset.isLand(*tile))
										goto failed;
									if (realm->hasFluid({row, column}))
										goto failed;
								}
							}

							thread_candidates.push_back(position);
							failed: continue;
						}

						std::unique_lock lock(candidates_mutex);
						candidates.insert(candidates.end(), thread_candidates.begin(), thread_candidates.end());
						--waiter;
					});
				}

				waiter.wait();
				candidate_timer.stop();

				if (!candidates.empty()) {
					std::default_random_engine town_rng(noise_seed + 1);
					std::sort(candidates.begin(), candidates.end());
					WorldGen::generateTown(realm, town_rng, choose(candidates, town_rng) + Position(pad + 1, 0), n, m, pad, noise_seed);
				}
			}
		}

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
				pool.add([realm, &waiter, &get_biome, &perlin, &params, noise_seed, row_min, row_max, col_min, col_max](ThreadPool &, size_t) {
					threadContext = {realm->getGame().shared_from_this(), static_cast<uint_fast32_t>(noise_seed - 1'000'000ul * row_min + col_min), row_min, row_max, col_min, col_max};
					for (Index row = row_min; row < row_max; ++row)
						for (Index column = col_min; column < col_max; ++column)
							get_biome(row, column).postgen(row, column, threadContext.rng, perlin, params);
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
			Timer pathmap_timer("RemakePathmap");
			realm->remakePathMap(range);
		}

		overworld_timer.stop();
		if (initial_generation)
			Timer::summary();
		Timer::clear();
	}
}
