#include <thread>

#include "ThreadContext.h"
#include "Tileset.h"
#include "biome/Biome.h"
#include "biome/Grassland.h"
#include "game/Game.h"
#include "lib/noise.h"
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
	void generateOverworld(const std::shared_ptr<Realm> &realm, size_t noise_seed, const WorldGenParams &params) {
		Timer overworld_timer("GenOverworld");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		const size_t regions_x = updiv(static_cast<size_t>(width), params.regionSize);
		const size_t regions_y = updiv(static_cast<size_t>(height), params.regionSize);
		const size_t thread_count = regions_x * regions_y;

		auto &biome_map = realm->biomeMap;
		biome_map->fill(Biome::GRASSLAND);

		std::vector<std::thread> threads;
		threads.reserve(thread_count);

		noise::module::Perlin p2;
		p2.SetSeed(noise_seed * 3 - 1);

		auto saved_noise = std::make_shared<double[]>(width * height);
		auto &tilemap1 = realm->tilemap1;
		auto &tilemap2 = realm->tilemap2;
		auto &tilemap3 = realm->tilemap3;
		const auto &tileset = realm->getTileset();

		tilemap1->reset();
		tilemap2->reset();
		tilemap3->reset();

		auto biomes = Biome::getMap(*realm, noise_seed, saved_noise);
		auto get_biome = [&](Index row, Index column) -> Biome & {
			return *biomes.at((*biome_map)(column, row));
		};

		for (Index row = 0; row < height; ++row) {
			for (Index column = 0; column < width; ++column) {
				constexpr double zoom = 1000;
				const double noise = p2.GetValue(row / zoom, column / zoom, 0.1);
				if (noise < -0.1)
					biome_map->tiles.at(realm->getIndex(row, column)) = Biome::VOLCANIC;
				else
					biome_map->tiles.at(realm->getIndex(row, column)) = Biome::GRASSLAND;
			}
		}

		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

		for (size_t thread_row = 0; thread_row < regions_y; ++thread_row) {
			const size_t row_min_long = thread_row * params.regionSize;
			const size_t row_max_long = std::min(static_cast<size_t>(height), (thread_row + 1) * params.regionSize);

			if (INT_MAX < row_min_long)
				throw std::runtime_error("Not going to generate an impossibly large world");

			for (size_t thread_col = 0; thread_col < regions_x; ++thread_col) {
				const size_t col_min_long = thread_col * params.regionSize;
				const size_t col_max_long = std::min(static_cast<size_t>(width), (thread_col + 1) * params.regionSize);

				if (INT_MAX < col_min_long)
					throw std::runtime_error("Not going to generate an impossibly large world");

				const auto col_min = static_cast<Index>(col_min_long);
				const auto col_max = static_cast<Index>(col_max_long);
				const auto row_min = static_cast<Index>(row_min_long);
				const auto row_max = static_cast<Index>(row_max_long);

				threads.emplace_back([&, row_min, row_max, col_min, col_max] {
					threadContext = {realm->getGame().shared_from_this(), noise_seed - 1'000'000ul * row_min + col_min, row_min, row_max, col_min, col_max};

					// Timer noise_timer("BiomeGeneration");
					for (auto row = row_min; row < row_max; ++row)
						for (auto column = col_min; column < col_max; ++column)
							get_biome(row, column).generate(row, column, threadContext.rng, perlin, params);
					// noise_timer.stop();

					// Timer resource_timer("Resources");
					std::vector<Index> resource_starts;
					resource_starts.reserve(width * height / 10);

					const auto ore_set = tileset.getCategoryIDs("base:category/orespawns"_id);

					// for (Index index = width * row_min, max = width * row_max; index < max; ++index)
					for (auto row = row_min; row < row_max; ++row) {
						for (auto column = col_min; column < col_max; ++column) {
							const Index index = realm->getIndex(row, column);
							if (ore_set.contains((*tilemap1)[index]))
								resource_starts.push_back(index);
						}
					}

					std::shuffle(resource_starts.begin(), resource_starts.end(), threadContext.rng);
					Game &game = realm->getGame();
					auto &ores = game.registry<OreRegistry>();

					auto add_resources = [&](double threshold, const Identifier &ore_name) {
						auto ore = ores.at(ore_name);
						for (size_t i = 0, max = resource_starts.size() / 1000; i < max; ++i) {
							const Index index = resource_starts.back();
							if (Grassland::THRESHOLD + threshold <= saved_noise[index])
								realm->add(TileEntity::create<OreDeposit>(game, *ore, realm->getPosition(index)));
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

		std::mutex candidates_mutex;

		constexpr int m = 26, n = 34, pad = 2;
		Timer land_timer("GetLand");
		const auto starts = tilemap1->getLand(m + pad * 2, n + pad * 2);
		land_timer.stop();
		constexpr size_t chunk_size = 512;

		if (!starts.empty()) {
			Timer candidate_timer("Candidates");

			std::vector<Index> candidates;
			candidates.reserve(starts.size() / 16);
			std::vector<std::thread> candidate_threads;
			const size_t chunk_max = updiv(starts.size(), chunk_size);
			candidate_threads.reserve(chunk_max);
			const auto &tiles1 = tilemap1->getTiles();

			std::mutex candidates_mutex;

			for (size_t chunk = 0; chunk < chunk_max; ++chunk) {
				realm->randomLand = choose(starts, rng);

				candidate_threads.emplace_back([&, chunk] {
					std::vector<Index> thread_candidates;

					for (size_t i = chunk * chunk_size, max = std::min((chunk + 1) * chunk_size, starts.size()); i < max; ++i) {
						const auto index = starts[i];
						const size_t row_start = index / tilemap1->width + pad;
						const size_t row_end = row_start + m;
						const size_t column_start = index % tilemap1->width + pad;
						const size_t column_end = column_start + n;

						// Prevent towns from spawning at the right edge and wrapping to the left
						if (static_cast<size_t>(width) <= column_end)
							continue;

						for (size_t row = row_start; row < row_end; row += 2) {
							for (size_t column = column_start; column < column_end; column += 2) {
								const Index index = row * tilemap1->width + column;
								if (!tileset.isLand(tiles1[index]))
									goto failed;
							}
						}
						thread_candidates.push_back(index);
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
				WorldGen::generateTown(realm, rng, choose(candidates, rng) + pad * (tilemap1->width + 1), n, m, pad, noise_seed);
		}

		Timer postgen_timer("Postgen");

		for (size_t thread_row = 0; thread_row < regions_y; ++thread_row) {
			const size_t row_min = thread_row * params.regionSize;
			const size_t row_max = std::min(static_cast<size_t>(height), (thread_row + 1) * params.regionSize);
			for (size_t thread_col = 0; thread_col < regions_y; ++thread_col) {
				const size_t col_min = thread_col * params.regionSize;
				const size_t col_max = std::min(static_cast<size_t>(width), (thread_col + 1) * params.regionSize);
				const auto col_min_index = static_cast<Index>(col_min);
				const auto col_max_index = static_cast<Index>(col_max);
				const auto row_min_index = static_cast<Index>(row_min);
				const auto row_max_index = static_cast<Index>(row_max);
				threads.emplace_back([realm, &get_biome, &perlin, &params, noise_seed, row_min, col_min, row_min_index, row_max_index, col_min_index, col_max_index] {
					threadContext = {realm->getGame().shared_from_this(), noise_seed - 1'000'000ul * row_min_index + col_min_index, row_min_index, row_max_index, col_min_index, col_max_index};
					for (Index row = row_min_index; row < row_max_index; ++row)
						for (Index column = col_min_index; column < col_max_index; ++column)
							get_biome(row, column).postgen(row, column, threadContext.rng, perlin, params);
				});
			}
		}

		for (std::thread &thread: threads)
			thread.join();

		postgen_timer.stop();

		Timer pathmap_timer("RemakePathmap");
		realm->remakePathMap();
		pathmap_timer.stop();

		overworld_timer.stop();
		Timer::summary();
		Timer::clear();
	}
}
