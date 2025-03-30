#include "game/Game.h"
#include "graphics/Tileset.h"
#include "realm/Realm.h"
#include "tileentity/ArcadeMachine.h"
#include "tileentity/Building.h"
#include "tileentity/Teleporter.h"
#include "ui/gl/Constants.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Arcade.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateArcade(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, Position entrance) {
		auto guard = realm->guardGeneration();
		realm->markGenerated(ChunkPosition{0, 0});
		Timer timer("GenerateArcade");

		realm->tileProvider.ensureAllChunks(ChunkPosition{0, 0});
		generateIndoors(realm, rng, parent_realm, width, height, entrance, -1, "base:tile/concrete_wall", "base:tile/arcade_floor");

		const auto &tileset = realm->getTileset();
		const auto &plants = tileset.getTilesByCategory("base:category/plants"_id);

		realm->setTile(Layer::Submerged, {1, 1}, choose(plants, rng), false);
		realm->setTile(Layer::Submerged, {1, width - 2}, choose(plants, rng), false);
		realm->setTile(Layer::Submerged, {height - 2, 1}, choose(plants, rng), false);
		realm->setTile(Layer::Submerged, {height - 2, width - 2}, choose(plants, rng), false);

		for (Index row = 1; row <= 3; row += 2) {
			for (Index column = 2; column < width - 2; ++column) {
				Place place{Position{row, column}, realm};

				static const std::array<std::tuple<Identifier, Identifier, int, int>, 3> machines{
					std::tuple{"base:tile/arcade_machine_green", "base:minigame/breakout",    600, 600},
					std::tuple{"base:tile/arcade_machine_red",   "base:minigame/flappy_bird", 512, 256},
					std::tuple{"base:tile/arcade_machine_blue",  "base:minigame/math",        600, 600},
				};

				const auto &[tilename, minigame_name, game_width, game_height] = choose(machines);

				TileEntity::spawn<ArcadeMachine>(place, tilename, place.position, minigame_name, game_width, game_height);
			}
		}
	}
}
