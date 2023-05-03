#include <iostream>

#include "Tileset.h"
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
		realm->setTile(1, {row, column}, "base:tile/dirt"_id);
		const double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.1);
		if (noise < -.95) {
			realm->setTile(2, {row, column}, "base:tile/cave_iron"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.85) {
			realm->setTile(2, {row, column}, "base:tile/cave_wall"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.825) {
			realm->setTile(2, {row, column}, "base:tile/cave_diamond"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.725) {
			realm->setTile(2, {row, column}, "base:tile/cave_wall"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.7) {
			realm->setTile(2, {row, column}, "base:tile/cave_gold"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.6) {
			realm->setTile(2, {row, column}, "base:tile/cave_wall"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.55) {
			realm->setTile(2, {row, column}, "base:tile/cave_copper"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.45) {
			realm->setTile(2, {row, column}, "base:tile/cave_wall"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.375) {
			realm->setTile(2, {row, column}, "base:tile/cave_coal"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < -.1) {
			realm->setTile(2, {row, column}, "base:tile/cave_wall"_id);
			realm->setTile(3, {row, column}, "base:tile/void"_id);
		} else if (noise < .1) {
			realm->setTile(2, {row, column}, "base:tile/cave_wall"_id);
		} else if (noise < .11) {
			realm->setTile(2, {row, column}, "base:tile/cave_iron"_id);
		} else if (noise < .1125) {
			realm->setTile(2, {row, column}, "base:tile/cave_diamond"_id);
		} else if (noise < .12) {
			realm->setTile(2, {row, column}, "base:tile/cave_copper"_id);
		} else if (noise < .1225) {
			realm->setTile(2, {row, column}, "base:tile/cave_gold"_id);
		} else if (noise < .13) {
			realm->setTile(2, {row, column}, "base:tile/cave_coal"_id);
		} else
			return true;
		return false;
	}

	void generateCaveFull(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed, const Position &exit_position, Position &entrance, RealmID parent_realm, const ChunkRange &range) {
		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

		const Index row_min = CHUNK_SIZE * range.topLeft.y;
		const Index row_max = CHUNK_SIZE * (range.bottomRight.y + 1) - 1;
		const Index column_min = CHUNK_SIZE * range.topLeft.x;
		const Index column_max = CHUNK_SIZE * (range.bottomRight.x + 1) - 1;

		std::vector<Position> inside;

		for (Index row = row_min; row <= row_max; ++row)
			for (Index column = column_min; column <= column_max; ++column)
				if (generateCaveTile(realm, row, column, perlin))
					inside.emplace_back(row, column);

		if (inside.empty())
			entrance = {0, 0};
		else
			entrance = choose(inside, rng);

		realm->add(TileEntity::create<Building>(realm->getGame(), "base:tile/ladder"_id, entrance, parent_realm, exit_position));
	}
}
