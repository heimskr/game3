#include "Tiles.h"
#include "realm/Realm.h"

namespace Game3 {
	std::unordered_set<TileID> OverworldTiles::landSet {
		SAND, LIGHT_GRASS, GRASS, GRASS_ALT1, GRASS_ALT2, ROAD, DIRT, TILES, STONE,
	};

	std::unordered_set<TileID> OverworldTiles::solidSet {
		TOWER_NW, TOWER_NE, TOWER_SW, TOWER_SE, TOWER_WE, TOWER_NS, TOWER_N, TOWER_S, HOUSE1, HOUSE2, HOUSE3, MARKET1, MARKET2, MARKET3, KEEP_NW, KEEP_NE, KEEP_SW, KEEP_SE, IRON_ORE, DIAMOND_ORE,
		COPPER_ORE, GOLD_ORE, COAL_ORE,
	};

	std::unordered_set<TileID> HouseTiles::solidSet {
		WALL_NW, WALL_WEN, WALL_NE, WALL_NS, WALL_SW, WALL_WES, WALL_SE, WALL_E, WALL_W, BOOKSHELF, PLANT1, PLANT2, PLANT3, STOCKPILE_W, STOCKPILE_E,
	};

	OverworldTiles overworldTiles;
	HouseTiles houseTiles;

	std::unordered_map<RealmID, std::shared_ptr<TileSet>> tileSets {
		{Realm::OVERWORLD, std::make_shared<OverworldTiles>()},
		{Realm::HOUSE,     std::make_shared<HouseTiles>()},
		{Realm::KEEP,      std::make_shared<HouseTiles>()},
	};
}
