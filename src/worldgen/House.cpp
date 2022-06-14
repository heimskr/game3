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
#include "worldgen/Carpet.h"
#include "worldgen/House.h"

namespace Game3::WorldGen {
	void generateHouse(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance) {
		Timer timer("GenerateHouse");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		for (int column = 1; column < width - 1; ++column) {
			realm->setLayer2(column, HouseTiles::WALL_WEN);
			realm->setLayer2(height - 1, column, HouseTiles::WALL_WES);
		}

		for (int row = 1; row < height - 1; ++row) {
			realm->setLayer2(row, 0, HouseTiles::WALL_NS);
			realm->setLayer2(row, width - 1, HouseTiles::WALL_NS);
		}

		for (int row = 0; row < height; ++row)
			for (int column = 0; column < width; ++column)
				realm->setLayer1(row, column, HouseTiles::FLOOR);

		realm->setLayer2(0, HouseTiles::WALL_NW);
		realm->setLayer2(width - 1, HouseTiles::WALL_NE);
		realm->setLayer2(width * (height - 1), HouseTiles::WALL_SW);
		realm->setLayer2(width * height - 1, HouseTiles::WALL_SE);

		const Index exit_index = width * height - 3;
		realm->setLayer2(exit_index - 1, HouseTiles::WALL_W);
		realm->setLayer2(exit_index,     HouseTiles::EMPTY);
		realm->setLayer2(exit_index + 1, HouseTiles::WALL_E);

		static std::array<TileID, 3> plants {HouseTiles::PLANT1, HouseTiles::PLANT2, HouseTiles::PLANT3};
		static std::array<TileID, 3> beds   {HouseTiles::BED1,   HouseTiles::BED2,   HouseTiles::BED3};
		static std::array<TileID, 2> doors  {HouseTiles::DOOR1,  HouseTiles::DOOR2};

		realm->setLayer2(width + 1, choose(plants, rng));
		realm->setLayer2(2 * width - 2, choose(plants, rng));
		realm->setLayer2((width - 1) * height - 2, choose(plants, rng));
		realm->setLayer2((width - 2) * height + 1, choose(plants, rng));

		std::array<Index, 2> edges {1, width - 2};
		const Position bed_position(2 + rng() % (height - 4), choose(edges, rng));
		realm->setLayer2(realm->getIndex(bed_position), choose(beds, rng));
		realm->extraData["bed"] = bed_position;

		auto door = TileEntity::create<Teleporter>(choose(doors, rng), realm->getPosition(exit_index), parent_realm->id, entrance);
		realm->add(door);

		const auto house_position = entrance - Position(1, 0);
		realm->spawn<Gatherer>(realm->getPosition(exit_index - width), Entity::VILLAGER1_ID, parent_realm->id, realm->id, house_position, parent_realm->closestTileEntity<Building>(house_position,
			[](const auto &building) { return building->tileID == OverworldTiles::KEEP_SW; }));

		switch(rng() % 2) {
			case 0: {
				const static std::array<std::string, 13> texts {
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

				for (Index index = width + 2; index < 2 * width - 2; ++index) {
					realm->setLayer2(index, HouseTiles::BOOKSHELF);
					realm->add(TileEntity::create<Sign>(HouseTiles::EMPTY, realm->getPosition(index), shuffled_texts.at((index - width - 2) % shuffled_texts.size()), "Bookshelf"));
				}
				break;
			}

			case 1: {
				auto chest = TileEntity::create<Chest>(0, realm->getPosition(width * 3 / 2), "Chest");
				chest->setInventory(4);
				realm->add(chest);
				break;
			}

			default:
				break;
		}

		WorldGen::generateCarpet(realm, rng);
	}
}
