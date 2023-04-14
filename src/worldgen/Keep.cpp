#include "Tileset.h"
#include "realm/Keep.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/Keep.h"

namespace Game3::WorldGen {
	void generateKeep(const std::shared_ptr<Keep> &realm, std::default_random_engine &rng, RealmID parent_realm, const Position &entrance) {
		Timer timer("GenerateKeep");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();
		for (int column = 1; column < width - 1; ++column) {
			realm->setLayer2(column, "base:tile/wall_we"_id);
			realm->setLayer2({height - 1, column}, "base:tile/wall_we"_id);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setLayer2({row, 0}, "base:tile/wall_ns"_id);
			realm->setLayer2({row, width - 1}, "base:tile/wall_ns"_id);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setLayer1({row, column}, "base:tile/floor"_id);

		realm->setLayer2(0, "base:tile/wall_se"_id);
		realm->setLayer2(width - 1, "base:tile/wall_sw"_id);
		realm->setLayer2(width * (height - 1), "base:tile/wall_ne"_id);
		realm->setLayer2(width * height - 1, "base:tile/wall_nw"_id);

		const Index exit_index = width * height - width / 2 - 1;
		realm->setLayer2(exit_index - 1, "base:tile/wall_w"_id);
		realm->setLayer2(exit_index,     "base:tile/empty"_id);
		realm->setLayer2(exit_index + 1, "base:tile/wall_e"_id);

		const auto &doors = realm->tilemap2->tileset->getTilesByCategory("base:category/doors");

		Game &game = realm->getGame();
		auto exit_door = TileEntity::create<Teleporter>(game, choose(doors, rng), realm->getPosition(exit_index), parent_realm, entrance);
		exit_door->extraData["exit"] = true;
		realm->add(exit_door);

		realm->setLayer2(Position(height - 2, 1), "base:tile/stockpile_w"_id);
		realm->setLayer2(Position(height - 2, 2), "base:tile/stockpile_e"_id);
		// TODO: the identifier here used to inexplicably be 48 so I'm putting in a silly tile temporarily to see whether it does anything.
		auto stockpile = TileEntity::create<Stockpile>(game, "base:tile/furnace"_id, Position(height - 2, 2));
		stockpile->setInventory(40);
		realm->add(stockpile);
		realm->stockpileInventory = stockpile->inventory;

		WorldGen::generateCarpet(realm, rng);
	}
}
