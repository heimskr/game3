#include "Tileset.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/ItemSpawner.h"
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

		auto set = [&](auto &&...args) { realm->setLayer2(std::forward<decltype(args)>(args)...); };

		set(width + 1, choose(Monomap::PLANTS, rng));
		set(2 * width - 2, choose(Monomap::PLANTS, rng));
		set(width * (height - 1) - 2, choose(Monomap::PLANTS, rng));
		set(width * (height - 2) + 1, choose(Monomap::PLANTS, rng));

		set(1, width / 2, Monomap::FURNACE);

		constexpr Index table_padding_x = 4;
		constexpr Index table_padding_y = 3;
		constexpr Index table_spacing = 4;

		const Index table_count = 1 + (height - 2 * table_padding_y) / (table_spacing + 1);
		const Index table_rows  = 1 + (table_count - 1) * table_spacing;
		const Index table_start = (height - table_rows) / 2;

		for (Index table = 0; table < table_count; ++table) {
			Index row = table_start + table * table_spacing;

			// Chairs at the left/right edges of the table
			set(row, table_padding_x, Monomap::CHAIR_W);
			set(row, width - table_padding_x - 1, Monomap::CHAIR_E);

			// Left/right edges of the table
			set(row, table_padding_x + 1, Monomap::TABLE_W);
			set(row, width - table_padding_x - 2, Monomap::TABLE_E);

			// Table interior + chairs above/below tables
			for (Index col = table_padding_x + 2; col < width - table_padding_x - 2; ++col) {
				set(row, col, Monomap::TABLE_WE);
				if (rng() % 3 == 0) {
					realm->spawn<ItemEntity>({row, col}, ItemStack(Item::MEAD));
					static const std::vector<ItemStack> spawnables {{Item::MEAD}};
					realm->add(TileEntity::create<ItemSpawner>(Position(row, col), 0.0001f, spawnables));
				}

				if (2 < table_spacing)
					set(row - 1, col, Monomap::CHAIR_N);
				if (3 < table_spacing)
					set(row + 1, col, Monomap::CHAIR_S);
			}
		}

		WorldGen::generateCarpet(realm, rng, 3);
	}
}
