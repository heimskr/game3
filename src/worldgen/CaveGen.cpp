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

		for (Index row = row_min; row <= row_max; ++row)
			for (Index column = column_min; column <= column_max; ++column)
				generateCaveTile(realm, row, column, perlin);
	}

	bool generateCaveTile(const std::shared_ptr<Realm> &realm, Index row, Index column, const noise::module::Perlin &perlin) {
		realm->setTile(Layer::Terrain, {row, column}, "base:tile/cave_dirt"_id, true);
		const double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.1);
		if (noise < -.95) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_iron"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.85) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.825) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_diamond"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.725) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.7) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_gold"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.6) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.55) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_copper"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.45) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.375) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_coal"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < -.1) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall"_id, true);
			realm->setTile(Layer::Highest, {row, column}, "base:tile/void"_id, false);
		} else if (noise < .1) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_wall"_id, true);
		} else if (noise < .11) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_iron"_id, true);
		} else if (noise < .1125) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_diamond"_id, true);
		} else if (noise < .12) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_copper"_id, true);
		} else if (noise < .1225) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_gold"_id, true);
		} else if (noise < .13) {
			realm->setTile(Layer::Objects, {row, column}, "base:tile/cave_coal"_id, true);
		} else
			return true;
		return false;
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

		for (Index row = row_min; row <= row_max; ++row)
			for (Index column = column_min; column <= column_max; ++column)
				if (generateCaveTile(realm, row, column, perlin))
					inside.emplace_back(row, column);

		if (inside.empty())
			entrance = {0, 0};
		else
			entrance = choose(inside, rng);

		realm->add(TileEntity::create<Building>(realm->getGame(), "base:tile/ladder"_id, entrance, parent_realm, exit_position));
		timer.stop();
		Timer::summary();
		Timer::clear();
	}
}
