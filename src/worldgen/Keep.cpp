#include "Tileset.h"
#include "realm/Keep.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/Keep.h"

namespace Game3::WorldGen {
	void generateKeep(const std::shared_ptr<Keep> &realm, std::default_random_engine &rng, RealmID parent_realm, Index width, Index height, const Position &entrance) {
		realm->markGenerated(0, 0);
		Timer timer("GenerateKeep");
		for (int column = 1; column < width - 1; ++column) {
			realm->setTile(2, {0, column}, "base:tile/wall_we"_id);
			realm->setTile(2, {height - 1, column}, "base:tile/wall_we"_id);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setTile(2, {row, 0}, "base:tile/wall_ns"_id);
			realm->setTile(2, {row, width - 1}, "base:tile/wall_ns"_id);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setTile(1, {row, column}, "base:tile/floor"_id);

		realm->setTile(2, {0, 0}, "base:tile/wall_se"_id);
		realm->setTile(2, {0, width - 1}, "base:tile/wall_sw"_id);
		realm->setTile(2, {height - 1, 0}, "base:tile/wall_ne"_id);
		realm->setTile(2, {height - 1, width - 1}, "base:tile/wall_nw"_id);

		const Position exit_position(height - 1, width / 2);
		realm->setTile(2, exit_position - Position(0, 1), "base:tile/wall_w"_id);
		realm->setTile(2, exit_position,     "base:tile/empty"_id);
		realm->setTile(2, exit_position + Position(0, 1), "base:tile/wall_e"_id);

		const auto &doors = realm->getTileset().getTilesByCategory("base:category/doors");

		Game &game = realm->getGame();
		auto exit_door = TileEntity::create<Teleporter>(game, choose(doors, rng), exit_position, parent_realm, entrance);
		exit_door->extraData["exit"] = true;
		realm->add(exit_door);

		realm->setTile(2, Position(height - 2, 1), "base:tile/stockpile_w"_id);
		realm->setTile(2, Position(height - 2, 2), "base:tile/stockpile_e"_id);
		// TODO: the identifier here used to inexplicably be 48 so I'm putting in a silly tile temporarily to see whether it does anything.
		auto stockpile = TileEntity::create<Stockpile>(game, "base:tile/furnace"_id, Position(height - 2, 2));
		stockpile->setInventory(40);
		realm->add(stockpile);
		realm->stockpileInventory = stockpile->inventory;

		WorldGen::generateCarpet(realm, rng, width, height);
	}
}
