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
			realm->setLayer2(column, Monomap::WALL_WE);
			realm->setLayer2(height - 1, column, Monomap::WALL_WE);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setLayer2(row, 0, Monomap::WALL_NS);
			realm->setLayer2(row, width - 1, Monomap::WALL_NS);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setLayer1(row, column, Monomap::FLOOR);

		realm->setLayer2(0, Monomap::WALL_SE);
		realm->setLayer2(width - 1, Monomap::WALL_SW);
		realm->setLayer2(width * (height - 1), Monomap::WALL_NE);
		realm->setLayer2(width * height - 1, Monomap::WALL_NW);

		const Index exit_index = width * height - width / 2 - 1;
		realm->setLayer2(exit_index - 1, Monomap::WALL_W);
		realm->setLayer2(exit_index,     Monomap::EMPTY);
		realm->setLayer2(exit_index + 1, Monomap::WALL_E);

		static std::array<TileID, 2> doors  {Monomap::DOOR1,  Monomap::DOOR2};

		auto exit_door = TileEntity::create<Teleporter>(choose(doors, rng), realm->getPosition(exit_index), parent_realm, entrance);
		exit_door->extraData["exit"] = true;
		realm->add(exit_door);

		realm->setLayer2(Position(height - 2, 1), Monomap::STOCKPILE_W);
		realm->setLayer2(Position(height - 2, 2), Monomap::STOCKPILE_E);
		auto stockpile = TileEntity::create<Stockpile>(48, Position(height - 2, 2));
		stockpile->setInventory(40);
		realm->add(stockpile);
		realm->stockpileInventory = stockpile->inventory;

		WorldGen::generateCarpet(realm, rng);
	}
}
