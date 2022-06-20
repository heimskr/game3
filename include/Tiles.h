#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Texture.h"
#include "Types.h"

namespace Game3 {
	struct TileSet {
		TileSet() = default;
		virtual ~TileSet() = default;
		virtual bool isLand(TileID) const = 0;
		virtual bool isWalkable(TileID id) const { return isLand(id) || !isSolid(id); }
		virtual bool isSolid(TileID) const = 0;
		virtual TileID getEmpty() const { return 0; }
		virtual const char * name() const = 0;
		virtual Texture & getTexture() = 0;
	};

	struct OverworldTiles: TileSet {
		constexpr static TileID EMPTY = 0;
		constexpr static TileID DEEPER_WATER = 6;
		constexpr static TileID DEEP_WATER   = 3;
		constexpr static TileID WATER = 2;
		constexpr static TileID SHALLOW_WATER = 1;
		constexpr static TileID SAND  = 4;
		constexpr static TileID STONE = 7;
		constexpr static TileID LIGHT_GRASS = 11;
		constexpr static TileID GRASS = 12;
		constexpr static TileID GRASS_ALT1 = 32;
		constexpr static TileID GRASS_ALT2 = 33;
		constexpr static TileID GRAY  = 5;
		constexpr static TileID ROAD  = 15;
		constexpr static TileID DIRT  = 16;
		constexpr static TileID TILES = 17;
		constexpr static TileID OIL   = 34;
		constexpr static TileID IRON_ORE    = 35;
		constexpr static TileID DIAMOND_ORE = 36;
		constexpr static TileID COPPER_ORE  = 37;
		constexpr static TileID GOLD_ORE    = 38;
		constexpr static TileID COAL_ORE    = 39;
		constexpr static TileID MARKET1     = 40;
		constexpr static TileID MARKET2     = 41;
		constexpr static TileID MARKET3     = 42;
		constexpr static TileID BLACKSMITH1 = 43;
		constexpr static TileID BLACKSMITH2 = 44;
		constexpr static TileID BLACKSMITH3 = 45;
		constexpr static TileID TOWER_NW = 50;
		constexpr static TileID TOWER_NE = 51;
		constexpr static TileID TOWER_SW = 52;
		constexpr static TileID TOWER_SE = 53;
		constexpr static TileID TOWER_WE = 54;
		constexpr static TileID TOWER_NS = 55;
		constexpr static TileID TOWER_N  = 56;
		constexpr static TileID TOWER_S  = 57;
		constexpr static TileID HOUSE1   = 60;
		constexpr static TileID HOUSE2   = 61;
		constexpr static TileID HOUSE3   = 62;
		constexpr static TileID TREE1    = 63;
		constexpr static TileID TREE2    = 64;
		constexpr static TileID TREE3    = 65;
		constexpr static TileID TREE0    = 66;
		constexpr static TileID KEEP_NW  = 70;
		constexpr static TileID KEEP_NE  = 71;
		constexpr static TileID MISSING  = 74;
		constexpr static TileID IRON_ORE_REGEN    = 75;
		constexpr static TileID DIAMOND_ORE_REGEN = 76;
		constexpr static TileID COPPER_ORE_REGEN  = 77;
		constexpr static TileID GOLD_ORE_REGEN    = 78;
		constexpr static TileID COAL_ORE_REGEN    = 79;
		constexpr static TileID KEEP_SW = 80;
		constexpr static TileID KEEP_SE = 81;

		static std::unordered_set<TileID> landSet;
		static std::unordered_set<TileID> solidSet;

		bool isLand(TileID tile) const override {
			return landSet.contains(tile);
		}

		bool isWalkable(TileID tile) const override {
			return landSet.contains(tile) || tile == SHALLOW_WATER || tile == EMPTY || tile == OIL;
		}

		bool isSolid(TileID tile) const override {
			return solidSet.contains(tile);
		}

		bool isResource(TileID tile) const {
			return OIL <= tile && tile <= COAL_ORE;
		}

		TileID getEmpty() const override { return EMPTY; }

		const char * name() const override {
			return "Overworld";
		}

		Texture & getTexture() override {
			return cacheTexture("resources/tileset2.png");
		}
	};

	struct HouseTiles: TileSet {
		constexpr static TileID EMPTY       =   0;
		constexpr static TileID CARPET1_NW  =   1;
		constexpr static TileID CARPET1_N   =   2;
		constexpr static TileID CARPET1_NE  =   3;
		constexpr static TileID CARPET1_W   =  29;
		constexpr static TileID CARPET1_C   =  30;
		constexpr static TileID CARPET1_E   =  31;
		constexpr static TileID CARPET1_SW  =  57;
		constexpr static TileID CARPET1_S   =  58;
		constexpr static TileID CARPET1_SE  =  59;
		constexpr static TileID CARPET2_NW  =   9;
		constexpr static TileID CARPET2_N   =  10;
		constexpr static TileID CARPET2_NE  =  11;
		constexpr static TileID CARPET2_W   =  37;
		constexpr static TileID CARPET2_C   =  38;
		constexpr static TileID CARPET2_E   =  39;
		constexpr static TileID CARPET2_SW  =  65;
		constexpr static TileID CARPET2_S   =  66;
		constexpr static TileID CARPET2_SE  =  67;
		constexpr static TileID CARPET3_NW  =  17;
		constexpr static TileID CARPET3_N   =  18;
		constexpr static TileID CARPET3_NE  =  19;
		constexpr static TileID CARPET3_W   =  45;
		constexpr static TileID CARPET3_C   =  46;
		constexpr static TileID CARPET3_E   =  47;
		constexpr static TileID CARPET3_SW  =  73;
		constexpr static TileID CARPET3_S   =  74;
		constexpr static TileID CARPET3_SE  =  75;
		constexpr static TileID WALL_NW     = 168;
		constexpr static TileID WALL_WEN    = 169;
		constexpr static TileID WALL_NE     = 170;
		constexpr static TileID WALL_E      = 172;
		constexpr static TileID WALL_W      = 174;
		constexpr static TileID WALL_NS     = 196;
		constexpr static TileID FLOOR       = 197;
		constexpr static TileID WALL_SW     = 224;
		constexpr static TileID WALL_WES    = 225;
		constexpr static TileID WALL_SE     = 226;
		constexpr static TileID COUNTER     = 252;
		constexpr static TileID COUNTER_W   = 254;
		constexpr static TileID COUNTER_WE  = 255;
		constexpr static TileID COUNTER_E   = 256;
		constexpr static TileID DOOR1       = 282;
		constexpr static TileID DOOR2       = 284;
		constexpr static TileID STOCKPILE_W = 286;
		constexpr static TileID STOCKPILE_E = 287;
		constexpr static TileID BED1        = 308;
		constexpr static TileID BED2        = 309;
		constexpr static TileID BED3        = 310;
		constexpr static TileID BOOKSHELF   = 311;
		constexpr static TileID PLANT1      = 340;
		constexpr static TileID PLANT2      = 341;
		constexpr static TileID PLANT3      = 342;
		constexpr static TileID ANVIL       = 397;
		constexpr static TileID FURNACE     = 398;

		constexpr static std::array<TileID, 3> BEDS {BED1, BED2, BED3};
		constexpr static std::array<TileID, 3> PLANTS {PLANT1, PLANT2, PLANT3};
		constexpr static std::array<TileID, 2> DOORS {DOOR1, DOOR2};

		static std::unordered_set<TileID> landSet;
		static std::unordered_set<TileID> solidSet;

		bool isLand(TileID tile) const override {
			return landSet.contains(tile);
		}

		bool isSolid(TileID tile) const override {
			return solidSet.contains(tile);
		}

		TileID getEmpty() const override { return EMPTY; }

		bool isBed(TileID tile) const {
			return tile == BED1 || tile == BED2 || tile == BED3;
		}

		const char * name() const override {
			return "House";
		}

		Texture & getTexture() override {
			return cacheTexture("resources/house.png");
		}
	};

	extern OverworldTiles overworldTiles;
	extern HouseTiles houseTiles;
	extern std::unordered_map<RealmID, std::shared_ptr<TileSet>> tileSets;
}
