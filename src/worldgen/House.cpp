#include "entity/Miner.h"
#include "game/Game.h"
#include "graphics/Tileset.h"
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
	void generateHouse(const std::shared_ptr<Realm> &realm, std::default_random_engine &rng, const std::shared_ptr<Realm> &parent_realm, Index width, Index height, Position entrance) {
		auto guard = realm->guardGeneration();
		realm->markGenerated(0, 0);
		Timer timer("GenerateHouse");

		realm->tileProvider.ensureAllChunks(ChunkPosition{0, 0});
		generateIndoors(realm, rng, parent_realm, width, height, entrance, -1);

		const auto &tileset = realm->getTileset();
		const auto &plants = tileset.getTilesByCategory("base:category/plants"_id);

		realm->setTile(Layer::Submerged, {1, 1}, choose(plants, rng), false);
		realm->setTile(Layer::Submerged, {1, width - 2}, choose(plants, rng), false);
		realm->setTile(Layer::Submerged, {height - 2, 1}, choose(plants, rng), false);
		realm->setTile(Layer::Submerged, {height - 2, width - 2}, choose(plants, rng), false);

		const auto &beds = tileset.getTilesByCategory("base:category/beds"_id);
		std::array<Index, 2> edges {1, width - 2};
		const Position bed_position(2 + rng() % (height - 4), choose(edges, rng));
		realm->setTile(Layer::Objects, bed_position, choose(beds, rng), false);
		realm->extraData["bed"] = bed_position;

		// const auto house_position = entrance - Position(1, 0);
		// realm->spawn<Miner>({exit_position.row - 1, exit_position.column}, parent_realm->id, realm->id, house_position, parent_realm->closestTileEntity<Building>(house_position,
		// 	[](const auto &building) { return building->tileID == "base:tile/keep_sw"_id; }));

		switch(std::uniform_int_distribution(0, 1)(rng)) {
			case 0: {
				std::array texts{
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
					"Use support libraries as appropriate.",
					"ha HA!\ntrapped in my furniture hellscape\nit's all so wrong and I made it just for you!\ngood luck!\n\n-- Louie Zong",
					"This is a work of nonfiction and any resemblance to actual events, locales or persons, living or dead, is entirely intentional. We are in your walls.",
					"Gangblanc, Gangblanc, give me your answer, do\nI'm half forswonk all for the code of you\nIt won't be a stylish shader\nI can't code up a trader\nBut you'll look cool without the tools\nOf a game engine built for three",
					"U18446744073709551615 is totally a valid molecule.",
					"Gangblanc is 1 cubic meter but a lot of it is hidden inside pocket dimensions so he appears to have a normal human volume.",
					"I've heard whispers of a vending machine deep beneath the surface that takes quarters... but what kind?",
					"4200200000001f16000000626173653a6d696e6967616d652f627265616b6f757404ffffffffffffffff",
					"worlds grow old and suns grow cold, and death we never can doubt\ntime's cold wind wailing down the past reminds us that all flesh is grass\nand history's lamps blow out",
					"cycles turn while the far stars burn\nand people and planets age\nlife's crown passes to younger lands\ntime sweeps dust of hope from her hands\nand turns another page",
					"but we who feel the weight of the wheel, when winter falls over our world\ncan hope for tomorrow and raise our eyes\nto a silver moon in the open skies and a single flag unfurled",
					"we know well what life can tell: if you will not perish, then grow\nand today our fragile flesh and steel have laid our hands on a vaster wheel\nwith all of the stars to know",
					"from all who tried out of history's tide, a salute for the team that won\nand the old earth smiles at her children's reach\nthe wave that carried us up the beach\nto reach for the shining sun",
				};

				std::ranges::shuffle(texts, rng);

				for (Index column = 2; column < width - 2; ++column) {
					realm->setTile(Layer::Objects, {1, column}, "base:tile/bookshelf"_id, false);
					TileEntity::spawn<Sign>(realm, "base:tile/empty"_id, Position(1, column), texts.at((column - 2) % texts.size()), "Bookshelf");
				}
				break;
			}

			case 1: {
				auto chest = TileEntity::spawn<Chest>(realm, Position(1, width / 2));
				assert(chest);
				chest->setInventory(30);
				break;
			}

			default:
				break;
		}

		WorldGen::generateCarpet(realm, rng, width, height);
	}
}
