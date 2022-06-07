#include "Tiles.h"
#include "game/Realm.h"

namespace Game3 {
	std::unordered_set<TileID> OverworldTiles::landSet {
		SAND, LIGHT_GRASS, GRASS, GRASS_ALT1, GRASS_ALT2, ROAD, DIRT,
	};

	std::unordered_set<TileID> OverworldTiles::solidSet {
		TOWER_NW, TOWER_NE, TOWER_SW, TOWER_SE, TOWER_WE, TOWER_NS, TOWER_N, TOWER_S, HOUSE1, HOUSE2, HOUSE3,
	};

	std::unordered_set<TileID> HouseTiles::solidSet {
		WALL_NW, WALL_WEN, WALL_NE, WALL_NS, WALL_SW, WALL_WES, WALL_SE, WALL_E, WALL_W,
	};

	OverworldTiles overworldTiles;
	HouseTiles houseTiles;

	std::unordered_map<RealmID, std::shared_ptr<TileSet>> tileSets {
		{Realm::OVERWORLD, std::make_shared<OverworldTiles>()},
		{Realm::HOUSE,     std::make_shared<HouseTiles>()},
	};
}
