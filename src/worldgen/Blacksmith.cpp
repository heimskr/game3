#include "Tiles.h"
#include "entity/Gatherer.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Sign.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Blacksmith.h"
#include "worldgen/Carpet.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateBlacksmith(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance) {
		Timer timer("GenerateBlacksmith");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		generateIndoors(realm, rng, parent_realm, entrance);
		const Index exit_index = width * height - 3;

		const auto house_position = entrance - Position(1, 0);
		realm->spawn<Gatherer>(realm->getPosition(exit_index - width), Entity::VILLAGER1_ID, parent_realm->id, realm->id, house_position, parent_realm->closestTileEntity<Building>(house_position,
			[](const auto &building) { return building->tileID == OverworldTiles::KEEP_SW; }));

		realm->setLayer2({1, 3}, HouseTiles::FURNACE);
		realm->setLayer2({1, 5}, HouseTiles::ANVIL);
	}
}
