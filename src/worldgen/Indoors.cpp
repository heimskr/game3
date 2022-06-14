#include "Tiles.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateIndoors(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance) {
		Timer timer("GenerateIndoors");
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

		const Index exit_index = width * height - 3;
		realm->setLayer2(exit_index - 1, HouseTiles::WALL_W);
		realm->setLayer2(exit_index,     HouseTiles::EMPTY);
		realm->setLayer2(exit_index + 1, HouseTiles::WALL_E);

		static std::array<TileID, 2> doors {HouseTiles::DOOR1, HouseTiles::DOOR2};
		auto door = TileEntity::create<Teleporter>(choose(doors, rng), realm->getPosition(exit_index), parent_realm->id, entrance);
		realm->add(door);
	}
}
