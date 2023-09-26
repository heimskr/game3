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
		auto pauser = realm->pauseUpdates();

		for (int column = 1; column < width - 1; ++column) {
			realm->setTile(Layer::Objects, {0, column}, "base:tile/wall_we"_id, false, true);
			realm->setTile(Layer::Objects, {height - 1, column}, "base:tile/wall_we"_id, false, true);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setTile(Layer::Objects, {row, 0}, "base:tile/wall_ns"_id, false, true);
			realm->setTile(Layer::Objects, {row, width - 1}, "base:tile/wall_ns"_id, false, true);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setTile(Layer::Terrain, {row, column}, "base:tile/floor"_id, false, true);

		realm->setTile(Layer::Objects, {0, 0}, "base:tile/wall_se"_id, false, true);
		realm->setTile(Layer::Objects, {0, width - 1}, "base:tile/wall_sw"_id, false, true);
		realm->setTile(Layer::Objects, {height - 1, 0}, "base:tile/wall_ne"_id, false, true);
		realm->setTile(Layer::Objects, {height - 1, width - 1}, "base:tile/wall_nw"_id, false, true);

		const Position exit_position(height - 1, width / 2);
		realm->setTile(Layer::Objects, exit_position - Position(0, 1), "base:tile/wall_w"_id, false, true);
		realm->setTile(Layer::Objects, exit_position,     "base:tile/empty"_id, false, true);
		realm->setTile(Layer::Objects, exit_position + Position(0, 1), "base:tile/wall_e"_id, false, true);

		const auto &doors = realm->getTileset().getTilesByCategory("base:category/doors");

		Game &game = realm->getGame();
		auto exit_door = TileEntity::create<Teleporter>(game, choose(doors, rng), exit_position, parent_realm, entrance);
		exit_door->extraData["exit"] = true;
		realm->add(exit_door);

		realm->setTile(Layer::Objects, Position(height - 2, 1), "base:tile/stockpile_w"_id, false, true);
		realm->setTile(Layer::Objects, Position(height - 2, 2), "base:tile/stockpile_e"_id, false, true);
		// TODO: the identifier here used to inexplicably be 48 so I'm putting in a silly tile temporarily to see whether it does anything.
		auto stockpile = TileEntity::create<Stockpile>(game, "base:tile/stockpile_e"_id, Position(height - 2, 2));
		assert(realm->add(stockpile));
		stockpile->setInventory(40);
		realm->stockpileInventory = stockpile->getInventory();

		WorldGen::generateCarpet(realm, rng, width, height);
	}
}
