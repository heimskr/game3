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

namespace Game3::WorldGen {
	void generateOverworld(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed, const WorldGenParams &params) {
		Timer overworld_timer("GenOverworld");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		auto &biome_map = realm->biomeMap;
		biome_map->fill(Biome::GRASSLAND);

		noise::module::Perlin p2;
		p2.SetSeed(noise_seed * 3 - 1);
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

		auto saved_noise = std::make_shared<double[]>(width * height);

		auto biomes = Biome::getMap(*realm, noise_seed, saved_noise);

		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

		auto &tilemap1 = realm->tilemap1;
		auto &tilemap2 = realm->tilemap2;
		auto &tilemap3 = realm->tilemap3;

		tilemap1->reset();
		tilemap2->reset();
		tilemap3->reset();

		auto get_biome = [&](Index row, Index column) -> Biome & {
			return *biomes.at((*biome_map)(column, row));
		};

		Timer noise_timer("BiomeGeneration");
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				get_biome(row, column).generate(row, column, rng, perlin, params);
		noise_timer.stop();

		constexpr static int m = 26, n = 34, pad = 2;
		Timer land_timer("GetLand");
		const auto starts = tilemap1->getLand(m + pad * 2, n + pad * 2);
		land_timer.stop();

		Timer resource_timer("Resources");

		std::vector<Index> resource_starts;
		resource_starts.reserve(width * height / 10);

		const auto &tileset = *tilemap1->tileset;
		const auto ore_set = tileset.getCategoryIDs("base:category/orespawns"_id);

		for (Index index = 0, max = width * height; index < max; ++index)
			if (ore_set.contains((*tilemap1)[index]))
				resource_starts.push_back(index);

		std::shuffle(resource_starts.begin(), resource_starts.end(), rng);
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
		resource_timer.stop();

		if (!starts.empty()) {
			realm->randomLand = choose(starts, rng);
			std::vector<Index> candidates;
			candidates.reserve(starts.size() / 16);
			Timer candidate_timer("Candidates");
			const auto &tiles1 = tilemap1->getTiles();
			for (const auto index: starts) {
				const size_t row_start = index / tilemap1->width + pad, row_end = row_start + m;
				const size_t column_start = index % tilemap1->width + pad, column_end = column_start + n;
				for (size_t row = row_start; row < row_end; row += 2)
					for (size_t column = column_start; column < column_end; column += 2) {
						const Index index = row * tilemap1->width + column;
						if (!tileset.isLand(tiles1[index]))
							goto failed;
					}
				candidates.push_back(index);
				failed:
				continue;
			}
			candidate_timer.stop();

			if (!candidates.empty())
				WorldGen::generateTown(realm, rng, choose(candidates, rng) + pad * (tilemap1->width + 1), n, m, pad, noise_seed);
		}

		Timer postgen_timer("Postgen");
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				get_biome(row, column).postgen(row, column, rng, perlin, params);
		postgen_timer.stop();

		overworld_timer.stop();
		// Timer::summary();
		Timer::clear();
	}
}
