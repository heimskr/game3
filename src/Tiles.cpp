#include "Tiles.h"

namespace Game3 {
	std::unordered_set<TileID> landSet {
		SAND, LIGHT_GRASS, GRASS, GRASS_ALT1, GRASS_ALT2, ROAD, DIRT,
	};

	std::unordered_set<TileID> solidSet {
		TOWER_NW, TOWER_NE, TOWER_SW, TOWER_SE, TOWER_WE, TOWER_NS, TOWER_N, TOWER_S, HOUSE1, HOUSE2, HOUSE3,
	};
}
