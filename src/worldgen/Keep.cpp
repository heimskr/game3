#include "Tiles.h"
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
			realm->setLayer2(column, HouseTiles::WALL_WEN);
			realm->setLayer2(height - 1, column, HouseTiles::WALL_WES);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setLayer2(row, 0, HouseTiles::WALL_NS);
			realm->setLayer2(row, width - 1, HouseTiles::WALL_NS);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setLayer1(row, column, HouseTiles::FLOOR);

		realm->setLayer2(0, HouseTiles::WALL_NW);
		realm->setLayer2(width - 1, HouseTiles::WALL_NE);
		realm->setLayer2(width * (height - 1), HouseTiles::WALL_SW);
		realm->setLayer2(width * height - 1, HouseTiles::WALL_SE);

		const Index exit_index = width * height - width / 2 - 1;
		realm->setLayer2(exit_index - 1, HouseTiles::WALL_W);
		realm->setLayer2(exit_index,     HouseTiles::EMPTY);
		realm->setLayer2(exit_index + 1, HouseTiles::WALL_E);

		static std::array<TileID, 2> doors  {HouseTiles::DOOR1,  HouseTiles::DOOR2};

		auto exit_door = TileEntity::create<Teleporter>(choose(doors, rng), realm->getPosition(exit_index), parent_realm, entrance);
		exit_door->extraData["exit"] = true;
		realm->add(exit_door);

		realm->setLayer2(Position(height - 2, 1), HouseTiles::STOCKPILE_W);
		realm->setLayer2(Position(height - 2, 2), HouseTiles::STOCKPILE_E);
		auto stockpile = TileEntity::create<Stockpile>(48, Position(height - 2, 2));
		stockpile->setInventory(40);
		realm->add(stockpile);
		realm->stockpileInventory = stockpile->inventory;

		WorldGen::generateCarpet(realm, rng);
	}
}
