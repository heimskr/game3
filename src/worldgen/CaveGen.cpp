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
	void generateCave(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, int noise_seed, Index exit_index, Position &entrance, RealmID parent_realm) {
		constexpr double noise_zoom = 20;

		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		noise::module::Perlin perlin;
		perlin.SetSeed(noise_seed);

		auto &game = realm->getGame();
		auto &tilemap1 = realm->tilemap1;
		auto &tilemap2 = realm->tilemap2;
		auto &tilemap3 = realm->tilemap3;

		tilemap1->reset();
		tilemap2->reset();
		tilemap3->reset();

		std::vector<Position> inside;

		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column) {
				realm->setLayer1({row, column}, "base:tile/dirt"_id);
				const double noise = perlin.GetValue(row / noise_zoom, column / noise_zoom, 0.1);
				if (noise < -.95) {
					realm->setLayer2({row, column}, "base:tile/cave_iron"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.85) {
					realm->setLayer2({row, column}, "base:tile/cave_wall"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.825) {
					realm->setLayer2({row, column}, "base:tile/cave_diamond"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.725) {
					realm->setLayer2({row, column}, "base:tile/cave_wall"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.7) {
					realm->setLayer2({row, column}, "base:tile/cave_gold"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.6) {
					realm->setLayer2({row, column}, "base:tile/cave_wall"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.55) {
					realm->setLayer2({row, column}, "base:tile/cave_copper"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.45) {
					realm->setLayer2({row, column}, "base:tile/cave_wall"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.375) {
					realm->setLayer2({row, column}, "base:tile/cave_coal"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < -.1) {
					realm->setLayer2({row, column}, "base:tile/cave_wall"_id);
					realm->setLayer3({row, column}, "base:tile/void"_id);
				} else if (noise < .1) {
					realm->setLayer2({row, column}, "base:tile/cave_wall"_id);
				} else if (noise < .11) {
					realm->setLayer2({row, column}, "base:tile/cave_iron"_id);
				} else if (noise < .1125) {
					realm->setLayer2({row, column}, "base:tile/cave_diamond"_id);
				} else if (noise < .12) {
					realm->setLayer2({row, column}, "base:tile/cave_copper"_id);
				} else if (noise < .1225) {
					realm->setLayer2({row, column}, "base:tile/cave_gold"_id);
				} else if (noise < .13) {
					realm->setLayer2({row, column}, "base:tile/cave_coal"_id);
				} else
					inside.emplace_back(row, column);
			}

		if (inside.empty())
			entrance = {0, 0};
		else
			entrance = choose(inside, rng);

		realm->add(TileEntity::create<Building>(game, "base:tile/ladder"_id, entrance, parent_realm, exit_index));
	}
}
