#include "Tiles.h"
#include "biome/Biome.h"
#include "biome/Grassland.h"
#include "lib/noise.h"
#include "realm/Realm.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Teleporter.h"
#include "tileentity/Tree.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Overworld.h"
#include "worldgen/Town.h"

namespace Game3::WorldGen {
	void generateOverworld(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed) {
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		auto &biome_map = realm->biomeMap;

		// TODO: choose biomes via noise
		biome_map->fill(Biome::GRASSLAND_ID);

		auto saved_noise = std::make_shared<double[]>(width * height);

		auto biomes = Biome::getMap(*realm, noise_seed, saved_noise);

		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

		auto &tilemap1 = realm->tilemap1;
		auto &tilemap2 = realm->tilemap2;
		auto &tilemap3 = realm->tilemap3;

		tilemap1->tiles.assign(tilemap1->tiles.size(), 0);
		tilemap2->tiles.assign(tilemap2->tiles.size(), 0);
		tilemap3->tiles.assign(tilemap3->tiles.size(), 0);

		auto get_biome = [&](Index row, Index column) -> Biome & {
			return *biomes.at((*biome_map)(column, row));
		};

		Timer noise_timer("BiomeGeneration");
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				get_biome(row, column).generate(row, column, rng, perlin);
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
			if (tilemap1->tiles[index] == Monomap::STONE)
				resource_starts.push_back(index);
		std::shuffle(resource_starts.begin(), resource_starts.end(), rng);
		auto add_resources = [&](double threshold, Ore ore) {
			for (size_t i = 0, max = resource_starts.size() / 1000; i < max; ++i) {
				const Index index = resource_starts.back();
				if (Grassland::THRESHOLD + threshold <= saved_noise[index])
					realm->add(TileEntity::create<OreDeposit>(ore, realm->getPosition(index)));
				resource_starts.pop_back();
			}
		};
		add_resources(1.0, Ore::Iron);
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
					if (!monomap.isLand(tiles1[index]))
						goto failed;
				}
			candidates.push_back(index);
			failed:
			continue;
		}
		candidate_timer.stop();

		if (!candidates.empty())
			WorldGen::generateTown(realm, rng, choose(candidates, rng) + pad * (tilemap1->width + 1), n, m, pad, noise_seed);

		Timer postgen_timer("Postgen");
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				get_biome(row, column).postgen(row, column, rng, perlin);
		postgen_timer.stop();

		Timer::summary();
		Timer::clear();
	}
}
