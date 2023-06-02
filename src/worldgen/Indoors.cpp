#include "Tileset.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	Position generateIndoors(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, const Position &entrance, Index door_x) {
		Timer timer("GenerateIndoors");

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

		const Position exit_position {height - 1, door_x == -1? width - 3 : door_x};
		realm->setTile(Layer::Objects, exit_position - Position(0, 1), "base:tile/wall_w"_id, false, true);
		realm->setTile(Layer::Objects, exit_position,                  "base:tile/empty"_id, false, true);
		realm->setTile(Layer::Objects, exit_position + Position(0, 1), "base:tile/wall_e"_id, false, true);

		const auto door_name = choose(realm->getTileset().getTilesByCategory("base:category/doors"), rng);
		auto door = TileEntity::create<Teleporter>(realm->getGame(), door_name, exit_position, parent_realm->id, entrance);
		realm->add(door);

		return exit_position;
	}
}
