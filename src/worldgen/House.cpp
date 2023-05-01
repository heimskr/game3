#include "Tileset.h"
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
#include "worldgen/House.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateHouse(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, const Position &entrance) {
		Timer timer("GenerateHouse");
		const Position exit_position = generateIndoors(realm, rng, parent_realm, width, height, entrance);

		const auto &tileset2 = realm->getTileset();
		const auto &plants = tileset2.getTilesByCategory("base:category/plants"_id);

		realm->setTile(2, {1, 1}, choose(plants, rng));
		realm->setTile(2, {1, width - 2}, choose(plants, rng));
		realm->setTile(2, {height - 1, 1}, choose(plants, rng));
		realm->setTile(2, {height - 1, width - 2}, choose(plants, rng));

		const auto &beds = tileset2.getTilesByCategory("base:category/beds"_id);
		std::array<Index, 2> edges {1, width - 2};
		const Position bed_position(2 + rng() % (height - 4), choose(edges, rng));
		realm->setTile(2, bed_position, choose(beds, rng));
		realm->extraData["bed"] = bed_position;

		const auto house_position = entrance - Position(1, 0);
		realm->spawn<Miner>({exit_position.row - 1, exit_position.column}, parent_realm->id, realm->id, house_position, parent_realm->closestTileEntity<Building>(house_position,
			[](const auto &building) { return building->tileID == "base:tile/keep_sw"_id; }));

		switch(rng() % 2) {
			case 0: {
				constexpr static std::array<const char *, 13> texts {
					"Express ideas directly in code.",
					"Write in ISO Standard C++.",
					"Express intent.",
					"Ideally, a program should be statically type safe.",
					"Prefer compile-time checking to run-time checking.",
					"What cannot be checked at compile time should be checkable at run time.",
					"Catch run-time errors early.",
					"Don't leak any resources.",
					"Don't waste time or space.",
					"Prefer immutable data to mutable data.",
					"Encapsulate messy constructs, rather than spreading through the code.",
					"Use supporting tools as appropriate.",
					"Use support libraries as appropriate."
				};

				auto shuffled_texts = texts;
				std::shuffle(shuffled_texts.begin(), shuffled_texts.end(), rng);

				// for (Index index = width + 2; index < 2 * width - 2; ++index) {
				for (Index column = 2; column < width - 2; ++column) {
					realm->setTile(2, {1, column}, "base:tile/bookshelf"_id);
					realm->add(TileEntity::create<Sign>(realm->getGame(), "base:tile/empty"_id, Position(1, column), shuffled_texts.at((column - 2) % shuffled_texts.size()), "Bookshelf"));
				}
				break;
			}

			case 1: {
				auto chest = TileEntity::create<Chest>(realm->getGame(), "base:tile/empty"_id, Position(1, width / 2), "Chest");
				chest->setInventory(4);
				realm->add(chest);
				break;
			}

			default:
				break;
		}

		WorldGen::generateCarpet(realm, rng, width, height);
	}
}
