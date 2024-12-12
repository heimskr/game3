#include "graphics/Tileset.h"
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
	void generateTavern(const RealmPtr &realm, std::default_random_engine &rng, const RealmPtr &parent_realm, Index width, Index height, const Position &entrance) {
		auto guard = realm->guardGeneration();
		realm->markGenerated(ChunkPosition{0, 0});
		realm->tileProvider.ensureAllChunks(ChunkPosition{0, 0});
		Timer timer("GenerateTavern");

		generateIndoors(realm, rng, parent_realm, width, height, entrance, width / 2);

		auto set = [&](auto &&...args) { realm->setTile(Layer::Objects, std::forward<decltype(args)>(args)..., false); };

		const auto &tileset = realm->getTileset();
		const auto &plants = tileset.getTilesByCategory("base:category/plants"_id);
		set(Position(1, 1), choose(plants, rng));
		set(Position(1, width - 2), choose(plants, rng));
		set(Position(height - 2, width - 2), choose(plants, rng));
		set(Position(height - 2, 1), choose(plants, rng));

		set(Position(1, width / 2), "base:tile/furnace"_id);

		constexpr Index table_padding_x = 4;
		constexpr Index table_padding_y = 3;
		constexpr Index table_spacing = 4;

		const Index table_count = 1 + (height - 2 * table_padding_y) / (table_spacing + 1);
		const Index table_rows  = 1 + (table_count - 1) * table_spacing;
		const Index table_start = (height - table_rows) / 2;

		for (Index table = 0; table < table_count; ++table) {
			Index row = table_start + table * table_spacing;

			// Chairs at the left/right edges of the table
			set(Position(row, table_padding_x), "base:tile/chair_w"_id);
			set(Position(row, width - table_padding_x - 1), "base:tile/chair_e"_id);

			// Left/right edges of the table
			set(Position(row, table_padding_x + 1), "base:tile/table_w"_id);
			set(Position(row, width - table_padding_x - 2), "base:tile/table_e"_id);

			// Table interior + chairs above/below tables
			for (Index col = table_padding_x + 2; col < width - table_padding_x - 2; ++col) {
				set(Position(row, col), "base:tile/table_we"_id);

				if (rng() % 3 == 0) {
					GamePtr game = realm->getGame();
					realm->spawn<ItemEntity>({row, col}, ItemStack::create(game, "base:item/mead"_id));
					TileEntity::spawn<ItemSpawner>(realm, Position(row, col), 10, 30, std::vector{
						ItemStack::create(game, "base:item/mead"_id),
					});
				}

				if (2 < table_spacing)
					set(Position(row - 1, col), "base:tile/chair_n"_id);

				if (3 < table_spacing)
					set(Position(row + 1, col), "base:tile/chair_s"_id);
			}
		}

		WorldGen::generateCarpet(realm, rng, width, height, 3);
	}
}
