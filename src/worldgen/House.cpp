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
#include "worldgen/House.h"
#include "worldgen/Indoors.h"

namespace Game3::WorldGen {
	void generateHouse(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, const Position &entrance) {
		Timer timer("GenerateHouse");
		const auto width  = realm->getWidth();
		const auto height = realm->getHeight();

		const Index exit_index = generateIndoors(realm, rng, parent_realm, entrance);

		realm->setLayer2(width + 1, choose(Monomap::PLANTS, rng));
		realm->setLayer2(2 * width - 2, choose(Monomap::PLANTS, rng));
		realm->setLayer2((width - 1) * height - 2, choose(Monomap::PLANTS, rng));
		realm->setLayer2((width - 2) * height + 1, choose(Monomap::PLANTS, rng));

		std::array<Index, 2> edges {1, width - 2};
		const Position bed_position(2 + rng() % (height - 4), choose(edges, rng));
		realm->setLayer2(realm->getIndex(bed_position), choose(Monomap::BEDS, rng));
		realm->extraData["bed"] = bed_position;

		const auto house_position = entrance - Position(1, 0);
		realm->spawn<Miner>(realm->getPosition(exit_index - width), Entity::VILLAGER1_ID, parent_realm->id, realm->id, house_position, parent_realm->closestTileEntity<Building>(house_position,
			[](const auto &building) { return building->tileID == Monomap::KEEP_SW; }));

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

				for (Index index = width + 2; index < 2 * width - 2; ++index) {
					realm->setLayer2(index, Monomap::BOOKSHELF);
					realm->add(TileEntity::create<Sign>(Monomap::EMPTY, realm->getPosition(index), shuffled_texts.at((index - width - 2) % shuffled_texts.size()), "Bookshelf"));
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
