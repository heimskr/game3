#include <iostream>

#include <libnoise/noise.h>

#include "Tiles.h"
#include "realm/Realm.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Teleporter.h"
#include "tileentity/Tree.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/Town.h"

namespace Game3::WorldGen {
	void generateOverworld(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed, double noise_zoom, double noise_threshold) {
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		noise::module::Perlin perlin;
		noise::module::Perlin forest_perlin;
		perlin.SetSeed(noise_seed);
		forest_perlin.SetSeed(-noise_seed * 3);

		auto &tilemap1 = realm->tilemap1;
		auto &tilemap2 = realm->tilemap2;
		auto &tilemap3 = realm->tilemap3;

		tilemap1->tiles.assign(tilemap1->tiles.size(), 0);
		tilemap2->tiles.assign(tilemap2->tiles.size(), 0);
		tilemap3->tiles.assign(tilemap3->tiles.size(), 0);

		static const std::vector<TileID> grasses {
			OverworldTiles::GRASS_ALT1, OverworldTiles::GRASS_ALT2,
			OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS, OverworldTiles::GRASS
		};

		auto saved_noise = std::make_unique<double[]>(width * height);

		Timer noise_timer("Noise");
		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column) {
				double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.666);
				saved_noise[row * width + column] = noise;
				if (noise < noise_threshold) {
					realm->setLayer1(row, column, OverworldTiles::DEEPER_WATER);
				} else if (noise < noise_threshold + 0.1) {
					realm->setLayer1(row, column, OverworldTiles::DEEP_WATER);
				} else if (noise < noise_threshold + 0.2) {
					realm->setLayer1(row, column, OverworldTiles::WATER);
				} else if (noise < noise_threshold + 0.3) {
					realm->setLayer1(row, column, OverworldTiles::SHALLOW_WATER);
				} else if (noise < noise_threshold + 0.4) {
					realm->setLayer1(row, column, OverworldTiles::SAND);
				} else if (noise < noise_threshold + 0.5) {
					realm->setLayer1(row, column, OverworldTiles::LIGHT_GRASS);
				} else if (0.8 < noise) {
					realm->setLayer1(row, column, OverworldTiles::STONE);
				} else {
					realm->setLayer1(row, column, choose(grasses, rng));
					const double forest_noise = forest_perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.5);
					if (0.5 < forest_noise)
						realm->add(TileEntity::create<Tree>(OverworldTiles::TREE1 + rand() % 3, OverworldTiles::TREE0, Position(row, column), Tree::MATURITY));
				}
			}
		noise_timer.stop();

		constexpr static int m = 26, n = 34, pad = 2;
		Timer land_timer("GetLand");
		auto starts = tilemap1->getLand(realm->type, m + pad * 2, n + pad * 2);
		if (starts.empty())
			throw std::runtime_error("Map has no land");
		land_timer.stop();

		Timer resource_timer("Resources");
		std::vector<Index> resource_starts;
		resource_starts.reserve(width * height / 10);
		for (Index index = 0, max = width * height; index < max; ++index)
			if (tilemap1->tiles[index] == OverworldTiles::STONE)
				resource_starts.push_back(index);
		std::shuffle(resource_starts.begin(), resource_starts.end(), rng);
		auto add_resources = [&](double threshold, Ore ore) {
			for (size_t i = 0, max = resource_starts.size() / 1000; i < max; ++i) {
				const Index index = resource_starts.back();
				if (noise_threshold + threshold <= saved_noise[index])
					realm->add(TileEntity::create<OreDeposit>(ore, realm->getPosition(index)));
				resource_starts.pop_back();
			}
		};
		add_resources(0.5, Ore::Iron);
		add_resources(0.5, Ore::Copper);
		add_resources(0.5, Ore::Gold);
		add_resources(0.5, Ore::Diamond);
		add_resources(0.5, Ore::Coal);
		// TODO: oil
		resource_timer.stop();

		realm->randomLand = choose(starts, rng);
		std::vector<Index> candidates;
		candidates.reserve(starts.size() / 16);
		Timer candidate_timer("Candidates");
		auto &tiles1 = tilemap1->tiles;
		for (const auto index: starts) {
			const size_t row_start = index / tilemap1->width + pad, row_end = row_start + m;
			const size_t column_start = index % tilemap1->width + pad, column_end = column_start + n;
			for (size_t row = row_start; row < row_end; row += 2)
				for (size_t column = column_start; column < column_end; column += 2) {
					const Index index = row * tilemap1->width + column;
					if (!overworldTiles.isLand(tiles1[index]))
						goto failed;
				}
			candidates.push_back(index);
			failed:
			continue;
		}
		candidate_timer.stop();

		std::cout << "Found " << candidates.size() << " candidate" << (candidates.size() == 1? "" : "s") << ".\n";
		if (!candidates.empty())
			WorldGen::generateTown(realm, rng, choose(candidates, rng) + pad * (tilemap1->width + 1), n, m, pad);

		Timer::summary();
		Timer::clear();
	}
}
