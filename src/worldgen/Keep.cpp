#include "graphics/Tileset.h"
#include "realm/Keep.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/Keep.h"

namespace Game3::WorldGen {
	void generateKeep(const std::shared_ptr<Keep> &realm, std::default_random_engine &rng, RealmID parent_realm, Index width, Index height, const Position &entrance) {
		Timer timer("GenerateKeep");
		realm->markGenerated(0, 0);
		realm->tileProvider.ensureAllChunks(ChunkPosition{0, 0});
		auto guard = realm->guardGeneration();

		for (int column = 1; column < width - 1; ++column) {
			realm->setTile(Layer::Objects, {0, column},          "base:tile/wooden_wall", true);
			realm->setTile(Layer::Objects, {height - 1, column}, "base:tile/wooden_wall", true);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setTile(Layer::Objects, {row, 0},         "base:tile/wooden_wall", true);
			realm->setTile(Layer::Objects, {row, width - 1}, "base:tile/wooden_wall", true);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setTile(Layer::Terrain, {row, column}, "base:tile/floor", false);

		realm->setTile(Layer::Objects, {0, 0},                  "base:tile/wooden_wall", true);
		realm->setTile(Layer::Objects, {0, width - 1},          "base:tile/wooden_wall", true);
		realm->setTile(Layer::Objects, {height - 1, 0},         "base:tile/wooden_wall", true);
		realm->setTile(Layer::Objects, {height - 1, width - 1}, "base:tile/wooden_wall", true);

		const Position exit_position(height - 1, width / 2);
		realm->setTile(Layer::Objects, exit_position - Position(0, 1), "base:tile/wooden_wall", true);
		realm->setTile(Layer::Objects, exit_position,                  "base:tile/empty", false);
		realm->setTile(Layer::Objects, exit_position + Position(0, 1), "base:tile/wooden_wall", true);

		const auto &doors = realm->getTileset().getTilesByCategory("base:category/doors");

		auto exit_door = TileEntity::spawn<Teleporter>(realm, choose(doors, rng), exit_position, parent_realm, entrance);
		assert(exit_door);
		exit_door->extraData["exit"] = true;

		realm->setTile(Layer::Objects, Position(height - 2, 1), "base:tile/stockpile_w", false);
		realm->setTile(Layer::Objects, Position(height - 2, 2), "base:tile/stockpile_e", false);
		// TODO: the identifier here used to inexplicably be 48 so I'm putting in a silly tile temporarily to see whether it does anything.
		auto stockpile = TileEntity::spawn<Stockpile>(realm, "base:tile/stockpile_e", Position(height - 2, 2));
		assert(stockpile);
		stockpile->setInventory(40);
		realm->stockpileInventory = stockpile->getInventory();

		WorldGen::generateCarpet(realm, rng, width, height);
	}
}
