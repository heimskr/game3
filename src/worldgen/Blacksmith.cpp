#include "Tiles.h"
#include "entity/Gatherer.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/CraftingStation.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Blacksmith.h"
#include "worldgen/Carpet.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateBlacksmith(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &, const Position &) {
		Timer timer("GenerateBlacksmith");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		realm->setLayer2({1, 3}, HouseTiles::FURNACE);
		realm->setLayer2({1, 5}, HouseTiles::ANVIL);
		realm->add(TileEntity::create<CraftingStation>(HouseTiles::FURNACE, Position(1, 3), CraftingStationType::Furnace));
		realm->add(TileEntity::create<CraftingStation>(HouseTiles::ANVIL,   Position(1, 5), CraftingStationType::Anvil));

		realm->setLayer2({height / 2, width / 2 - 1}, HouseTiles::COUNTER_W);
		realm->setLayer2({height / 2, width / 2},     HouseTiles::COUNTER_WE);
		realm->setLayer2({height / 2, width / 2 + 1}, HouseTiles::COUNTER_E);

		std::array<Index, 2> edges {1, width - 2};
		const Position bed_position(1, choose(edges, rng));
		realm->setLayer2(realm->getIndex(bed_position), choose(HouseTiles::BEDS, rng));
		realm->extraData["bed"] = bed_position;

		realm->spawn<Entity>({height / 2 - 1, width / 2}, Entity::BLACKSMITH_ID, Entity::GENERIC_TYPE);
	}
}
