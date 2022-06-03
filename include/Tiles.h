#pragma once

#include "Types.h"

namespace Game3 {
	constexpr static TileID EMPTY = 0;
	constexpr static TileID DEEPER_WATER = 6;
	constexpr static TileID DEEP_WATER = 3;
	constexpr static TileID WATER = 2;
	constexpr static TileID SHALLOW_WATER = 1;
	constexpr static TileID SAND = 4;
	constexpr static TileID LIGHT_GRASS = 11;
	constexpr static TileID GRASS = 12;
	constexpr static TileID GRASS_ALT1 = 40;
	constexpr static TileID GRASS_ALT2 = 41;
	constexpr static TileID GRAY = 5;
	constexpr static TileID ROAD = 15;
	constexpr static TileID DIRT = 16;
	constexpr static TileID TOWER_NW = 50;
	constexpr static TileID TOWER_NE = 51;
	constexpr static TileID TOWER_SW = 52;
	constexpr static TileID TOWER_SE = 53;
	constexpr static TileID TOWER_WE = 54;
	constexpr static TileID TOWER_NS = 55;
	constexpr static TileID TOWER_N = 56;
	constexpr static TileID TOWER_S = 57;
	constexpr static TileID HOUSE1 = 60;
	constexpr static TileID HOUSE2 = 61;
	constexpr static TileID HOUSE3 = 62;

	static inline bool isLand(TileID tile) {
		switch (tile) {
			case SAND:
			case LIGHT_GRASS:
			case GRASS:
			case GRASS_ALT1:
			case GRASS_ALT2:
				return true;
			default:
				return false;
		}
	}
}
