#include "Tiles.h"
#include "entity/Miner.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Sign.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/Tavern.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateTavern(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance) {
		Timer timer("GenerateTavern");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		const Index exit_index = generateIndoors(realm, rng, parent_realm, entrance, width / 2);

		realm->setLayer2(width + 1, choose(Monomap::PLANTS, rng));
		realm->setLayer2(2 * width - 2, choose(Monomap::PLANTS, rng));
		realm->setLayer2(width * (height - 1) - 2, choose(Monomap::PLANTS, rng));
		realm->setLayer2(width * (height - 2) + 1, choose(Monomap::PLANTS, rng));

		// std::array<Index, 2> edges {1, width - 2};
		// const Position bed_position(2 + rng() % (height - 4), choose(edges, rng));
		// realm->setLayer2(realm->getIndex(bed_position), choose(Monomap::BEDS, rng));

		// const auto building_position = entrance - Position(1, 0);
		// realm->spawn<Miner>(realm->getPosition(exit_index - width), Entity::VILLAGER1_ID, parent_realm->id, realm->id, building_position, parent_realm->closestTileEntity<Building>(building_position,
		// 	[](const auto &building) { return building->tileID == Monomap::KEEP_SW; }));

		WorldGen::generateCarpet(realm, rng);
	}
}
