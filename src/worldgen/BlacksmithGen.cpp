#include "Tiles.h"
#include "entity/Blacksmith.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/CraftingStation.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/BlacksmithGen.h"
#include "worldgen/Carpet.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateBlacksmith(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance) {
		Timer timer("GenerateBlacksmith");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		generateIndoors(realm, rng, parent_realm, entrance);

		realm->setLayer2({1, 3}, Monomap::FURNACE);
		realm->setLayer2({1, 5}, Monomap::ANVIL);
		realm->add(TileEntity::create<CraftingStation>(Monomap::FURNACE, Position(1, 3), CraftingStationType::Furnace));
		realm->add(TileEntity::create<CraftingStation>(Monomap::ANVIL,   Position(1, 5), CraftingStationType::Anvil));
		realm->extraData["furnace"] = Position(1, 3);
		realm->extraData["anvil"]   = Position(1, 5);

		realm->setLayer2({height / 2, width / 2 - 1}, Monomap::COUNTER_W);
		realm->setLayer2({height / 2, width / 2},     Monomap::COUNTER_WE);
		realm->setLayer2({height / 2, width / 2 + 1}, Monomap::COUNTER_E);
		realm->extraData["counter"] = Position(height / 2 - 1, width / 2);

		std::array<Index, 2> edges {1, width - 2};
		const Position bed_position(1, choose(edges, rng));
		realm->setLayer2(realm->getIndex(bed_position), choose(Monomap::BEDS, rng));
		realm->extraData["bed"] = bed_position;

		const Position building_position = entrance - Position(1, 0);
		realm->spawn<Blacksmith>(bed_position, Entity::BLACKSMITH_ID, parent_realm->id, realm->id, building_position, parent_realm->closestTileEntity<Building>(building_position,
			[](const auto &building) { return building->tileID == Monomap::KEEP_SW; }));
	}
}
