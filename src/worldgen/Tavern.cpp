#include "Tiles.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
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

		generateIndoors(realm, rng, parent_realm, entrance, width / 2);

		auto set = [&](auto &&...args) { realm->setLayer2(args...); };

		set(width + 1, choose(Monomap::PLANTS, rng));
		set(2 * width - 2, choose(Monomap::PLANTS, rng));
		set(width * (height - 1) - 2, choose(Monomap::PLANTS, rng));
		set(width * (height - 2) + 1, choose(Monomap::PLANTS, rng));

		set(1, width / 2, Monomap::FURNACE);

		constexpr Index table_padding_x = 4;
		constexpr Index table_padding_y = 3;
		constexpr Index table_spacing = 4;

		const Index table_count = 1 + (height - table_padding_y - table_padding_y) / (table_spacing + 1);
		const Index table_rows  = 1 + (table_count - 1) * table_spacing;
		const Index table_start = (height - table_rows) / 2;

		for (Index table = 0; table < table_count; ++table) {
			Index row = table_start + table * table_spacing;
			set(row, table_padding_x, Monomap::CHAIR_W);
			set(row, width - table_padding_x - 1, Monomap::CHAIR_E);
			set(row, table_padding_x + 1, Monomap::TABLE_W);
			set(row, width - table_padding_x - 2, Monomap::TABLE_E);

			for (Index col = table_padding_x + 2; col < width - table_padding_x - 2; ++col) {
				set(row, col, Monomap::TABLE_WE);
				if (rng() % 3 == 0)
					realm->spawn<ItemEntity>({row, col}, ItemStack(Item::MEAD));
				if (2 < table_spacing)
					set(row - 1, col, Monomap::CHAIR_N);
				if (3 < table_spacing)
					set(row + 1, col, Monomap::CHAIR_S);
			}
		}

		// std::array<Index, 2> edges {1, width - 2};
		// const Position bed_position(2 + rng() % (height - 4), choose(edges, rng));
		// realm->setLayer2(realm->getIndex(bed_position), choose(Monomap::BEDS, rng));

		// const auto building_position = entrance - Position(1, 0);
		// realm->spawn<Miner>(realm->getPosition(exit_index - width), Entity::VILLAGER1_ID, parent_realm->id, realm->id, building_position, parent_realm->closestTileEntity<Building>(building_position,
		// 	[](const auto &building) { return building->tileID == Monomap::KEEP_SW; }));

		WorldGen::generateCarpet(realm, rng, 3);
	}
}
