#include <thread>

#include "ThreadContext.h"
#include "Tileset.h"
#include "biome/Biome.h"
#include "biome/Grassland.h"
#include "game/Game.h"
#include "lib/noise.h"
#include "realm/Overworld.h"
#include "realm/Realm.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Teleporter.h"
#include "tileentity/Tree.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/Town.h"
#include "worldgen/WorldGen.h"

namespace Game3::WorldGen {
	void generateOverworld(const std::shared_ptr<Realm> &realm, size_t noise_seed, const WorldGenParams &params, const ChunkRange &range, bool initial_generation) {
		Timer overworld_timer("GenOverworld");

		const auto width  = range.tileWidth();
		const auto height = range.tileHeight();

		auto &provider = realm->tileProvider;

		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				provider.ensureAllChunks(ChunkPosition{x, y});

		const size_t regions_x = updiv(width,  CHUNK_SIZE);
		const size_t regions_y = updiv(height, CHUNK_SIZE);
		const size_t thread_count = regions_x * regions_y;

		std::vector<std::thread> threads;
		threads.reserve(thread_count);

		noise::module::Perlin p2;
		p2.SetSeed(noise_seed * 3 - 1);

		const auto &tileset = realm->getTileset();

		auto biomes = Biome::getMap(*realm, noise_seed);
		auto get_biome = [&](Index row, Index column) -> Biome & {
			if (auto biome_type = provider.copyBiomeType(row, column))
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
			for (Index column = range_column_min; column < range_column_max; ++column) {
				const double noise = std::min(1., std::max(-1., p2.GetValue(row / params.biomeZoom, column / params.biomeZoom, 0.0) * 5.));
				auto &type = provider.findBiomeType({row, column});
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

		const GamePtr game_ptr = realm->getGame().shared_from_this();

		for (int32_t y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (int32_t x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				provider.ensureAllChunks(ChunkPosition{x, y});

		for (size_t thread_row = 0; thread_row < regions_y; ++thread_row) {
			const Index row_min = range_row_min + thread_row * CHUNK_SIZE;
			// Compare with <, not <=
			const Index row_max = row_min + CHUNK_SIZE;

			for (size_t thread_col = 0; thread_col < regions_x; ++thread_col) {
				const Index col_min = range_column_min + thread_col * CHUNK_SIZE;
				// Compare with <, not <=
				const Index col_max = col_min + CHUNK_SIZE;

				threads.emplace_back([&, game_ptr, row_min, row_max, col_min, col_max] {
					threadContext = {game_ptr, noise_seed - 1'000'000ul * row_min + col_min, row_min, row_max, col_min, col_max};

					std::vector<double> saved_noise((row_max - row_min) * (col_max - col_min));

					size_t noise_index = 0;

					// Timer noise_timer("BiomeGeneration");
					for (auto row = row_min; row < row_max; ++row)
						for (auto column = col_min; column < col_max; ++column)
							saved_noise[noise_index++] = get_biome(row, column).generate(row, column, threadContext.rng, perlin, params);
					// noise_timer.stop();

					// Timer resource_timer("Resources");
					std::vector<Position> resource_starts;
					resource_starts.reserve(width * height / 10);

					const auto ore_set = tileset.getCategoryIDs("base:category/orespawns"_id);

					for (auto row = row_min; row < row_max; ++row)
						for (auto column = col_min; column < col_max; ++column)
							if (ore_set.contains(realm->getTile(1, {row, column})))
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
				});
			}

			// Timer land_timer("GetLand");
			// land_timer.stop();
		}

		for (std::thread &thread: threads)
			thread.join();

		threads.clear();

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
				std::vector<std::thread> candidate_threads;
				const size_t chunk_max = updiv(starts.size(), chunk_size);
				candidate_threads.reserve(chunk_max);

				std::mutex candidates_mutex;

				for (size_t chunk = 0; chunk < chunk_max; ++chunk) {
					realm->randomLand = choose(starts, rng);

					candidate_threads.emplace_back([&, chunk] {
						std::vector<Position> thread_candidates;

						for (size_t i = chunk * chunk_size, max = std::min((chunk + 1) * chunk_size, starts.size()); i < max; ++i) {
							const auto position = starts[i];
							const Index row_start = position.row + pad;
							const Index row_end = row_start + m;
							const Index column_start = position.column + pad;
							const Index column_end = column_start + n;

							for (Index row = row_start; row < row_end; row += 2) {
								for (Index column = column_start; column < column_end; column += 2) {
									// const Index index = row * tilemap1->width + column;
									if (auto tile = provider.tryTile(1, {row, column}); !tile || !tileset.isLand(*tile))
										goto failed;
								}
							}
							thread_candidates.push_back(position);
							failed: continue;
						}

						std::unique_lock lock(candidates_mutex);
						candidates.insert(candidates.end(), thread_candidates.begin(), thread_candidates.end());
					});
				}

				for (std::thread &thread: candidate_threads)
					thread.join();

				candidate_timer.stop();

				if (!candidates.empty())
					WorldGen::generateTown(realm, rng, choose(candidates, rng) + Position(pad + 1, 0), n, m, pad, noise_seed);
			}
		}

		Timer postgen_timer("Postgen");

		for (size_t thread_row = 0; thread_row < regions_y; ++thread_row) {
			const Index row_min = range_row_min + thread_row * CHUNK_SIZE;
			// Compare with <, not <=
			const Index row_max = row_min + CHUNK_SIZE;
			for (size_t thread_col = 0; thread_col < regions_y; ++thread_col) {
				const Index col_min = range_column_min + thread_col * CHUNK_SIZE;
				// Compare with <, not <=
				const Index col_max = col_min + CHUNK_SIZE;
				threads.emplace_back([realm, &get_biome, &perlin, &params, noise_seed, row_min, row_max, col_min, col_max] {
					threadContext = {realm->getGame().shared_from_this(), noise_seed - 1'000'000ul * row_min + col_min, row_min, row_max, col_min, col_max};
					for (Index row = row_min; row < row_max; ++row)
						for (Index column = col_min; column < col_max; ++column)
							get_biome(row, column).postgen(row, column, threadContext.rng, perlin, params);
				});
			}
		}

		for (std::thread &thread: threads)
			thread.join();

		postgen_timer.stop();

		if (initial_generation) {
			std::dynamic_pointer_cast<Overworld>(realm)->worldgenParams = params;
			Timer pathmap_timer("RemakePathmap");
			realm->remakePathMap();
		}

		overworld_timer.stop();
		Timer::summary();
		Timer::clear();
	}
}
