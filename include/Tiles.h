#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Types.h"

namespace Game3 {
	struct TileSet {
		TileSet() = default;
		virtual ~TileSet() = default;
		virtual bool isLand(TileID) const = 0;
		virtual bool isSolid(TileID) const = 0;
		virtual TileID getEmpty() const { return 0; }
		virtual const char * name() const = 0;
	};

	struct OverworldTiles: TileSet {
		constexpr static TileID EMPTY = 0;
		constexpr static TileID DEEPER_WATER = 6;
		constexpr static TileID DEEP_WATER = 3;
		constexpr static TileID WATER = 2;
		constexpr static TileID SHALLOW_WATER = 1;
		constexpr static TileID SAND = 4;
		constexpr static TileID LIGHT_GRASS = 11;
		constexpr static TileID GRASS = 12;
		constexpr static TileID GRASS_ALT1 = 32;
		constexpr static TileID GRASS_ALT2 = 33;
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

		static std::unordered_set<TileID> landSet;
		static std::unordered_set<TileID> solidSet;

		bool isLand(TileID tile) const override {
			return landSet.contains(tile);
		}

		bool isSolid(TileID tile) const override {
			return solidSet.contains(tile);
		}

		const char * name() const override {
			return "Overworld";
		}
	};

	struct HouseTiles: TileSet {
		constexpr static TileID EMPTY    = 175;
		constexpr static TileID WALL_NW  = 168;
		constexpr static TileID WALL_WEN = 169;
		constexpr static TileID WALL_NE  = 170;
		constexpr static TileID WALL_E   = 172;
		constexpr static TileID WALL_W   = 174;
		constexpr static TileID WALL_NS  = 196;
		constexpr static TileID FLOOR    = 197;
		constexpr static TileID WALL_SW  = 224;
		constexpr static TileID WALL_WES = 225;
		constexpr static TileID WALL_SE  = 226;
		constexpr static TileID DOOR     = 284;

		static std::unordered_set<TileID> solidSet;

		bool isLand(TileID) const override {
			return true;
		}

		bool isSolid(TileID tile) const override {
			return solidSet.contains(tile);
		}

		TileID getEmpty() const override { return EMPTY; }

		const char * name() const override {
			return "House";
		}
	};

	extern OverworldTiles overworldTiles;
	extern HouseTiles houseTiles;
	extern std::unordered_map<RealmID, std::shared_ptr<TileSet>> tileSets;
}
