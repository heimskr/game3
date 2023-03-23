#include "Tiles.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	Index generateIndoors(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance, Index door_pos) {
		Timer timer("GenerateIndoors");
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

		const Index exit_index = door_pos == -1? width * height - 3 : width * (height - 1) + door_pos;
		realm->setLayer2(exit_index - 1, Monomap::WALL_W);
		realm->setLayer2(exit_index,     Monomap::EMPTY);
		realm->setLayer2(exit_index + 1, Monomap::WALL_E);

		auto door = TileEntity::create<Teleporter>(choose(Monomap::DOORS, rng), realm->getPosition(exit_index), parent_realm->id, entrance);
		realm->add(door);

		return exit_index;
	}
}
