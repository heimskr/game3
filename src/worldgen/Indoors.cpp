#include "Tileset.h"
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
			realm->setLayer2(column, "base:tile/wall_we"_id);
			realm->setLayer2(height - 1, column, "base:tile/wall_we"_id);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setLayer2(row, 0, "base:tile/wall_ns"_id);
			realm->setLayer2(row, width - 1, "base:tile/wall_ns"_id);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setLayer1(row, column, "base:tile/floor"_id);

		realm->setLayer2(0, "base:tile/wall_se"_id);
		realm->setLayer2(width - 1, "base:tile/wall_sw"_id);
		realm->setLayer2(width * (height - 1), "base:tile/wall_ne"_id);
		realm->setLayer2(width * height - 1, "base:tile/wall_nw"_id);

		const Index exit_index = door_pos == -1? width * height - 3 : width * (height - 1) + door_pos;
		realm->setLayer2(exit_index - 1, "base:tile/wall_w"_id);
		realm->setLayer2(exit_index,     "base:tile/empty"_id);
		realm->setLayer2(exit_index + 1, "base:tile/wall_e"_id);

		const auto &door_name = choose(realm->tilemap2->tileset->getTilesByCategory("base:category/doors"), rng);
		auto door = TileEntity::create<Teleporter>(realm->getGame(), door_name, realm->getPosition(exit_index), parent_realm->id, entrance);
		realm->add(door);

		return exit_index;
	}
}
