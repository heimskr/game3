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

	struct Monomap: TileSet {
		constexpr static TileID EMPTY = 0;
		constexpr static TileID CARPET1_NW = 1;
		constexpr static TileID CARPET1_N = 2;
		constexpr static TileID CARPET1_NE = 3;
		constexpr static TileID CARPET1_W = 33;
		constexpr static TileID CARPET1_C = 34;
		constexpr static TileID CARPET1_E = 35;
		constexpr static TileID CARPET1_SW = 65;
		constexpr static TileID CARPET1_S = 66;
		constexpr static TileID CARPET1_SE = 67;
		constexpr static TileID CARPET2_NW = 9;
		constexpr static TileID CARPET2_N = 10;
		constexpr static TileID CARPET2_NE = 11;
		constexpr static TileID CARPET2_W = 41;
		constexpr static TileID CARPET2_C = 42;
		constexpr static TileID CARPET2_E = 43;
		constexpr static TileID CARPET2_SW = 73;
		constexpr static TileID CARPET2_S = 74;
		constexpr static TileID CARPET2_SE = 75;
		constexpr static TileID CARPET3_NW = 17;
		constexpr static TileID CARPET3_N = 18;
		constexpr static TileID CARPET3_NE = 19;
		constexpr static TileID CARPET3_W = 49;
		constexpr static TileID CARPET3_C = 50;
		constexpr static TileID CARPET3_E = 51;
		constexpr static TileID CARPET3_SW = 81;
		constexpr static TileID CARPET3_S = 82;
		constexpr static TileID CARPET3_SE = 83;
		constexpr static TileID WALL_NW = 192;
		constexpr static TileID WALL_WEN = 193;
		constexpr static TileID WALL_NE = 194;
		constexpr static TileID WALL_E = 196;
		constexpr static TileID WALL_W = 198;
		constexpr static TileID WALL_NS = 224;
		constexpr static TileID FLOOR = 225;
		constexpr static TileID WALL_SW = 256;
		constexpr static TileID WALL_WES = 257;
		constexpr static TileID WALL_SE = 258;
		constexpr static TileID COUNTER = 288;
		constexpr static TileID COUNTER_W = 290;
		constexpr static TileID COUNTER_WE = 291;
		constexpr static TileID COUNTER_E = 292;
		constexpr static TileID DOOR1 = 322;
		constexpr static TileID DOOR2 = 324;
		constexpr static TileID STOCKPILE_W = 326;
		constexpr static TileID STOCKPILE_E = 327;
		constexpr static TileID BED1 = 352;
		constexpr static TileID BED2 = 353;
		constexpr static TileID BED3 = 354;
		constexpr static TileID BOOKSHELF = 355;
		constexpr static TileID PLANT1 = 388;
		constexpr static TileID PLANT2 = 389;
		constexpr static TileID PLANT3 = 390;
		constexpr static TileID ANVIL = 453;
		constexpr static TileID FURNACE = 454;
		constexpr static TileID DEEPER_WATER = 486;
		constexpr static TileID DEEP_WATER = 483;
		constexpr static TileID WATER = 482;
		constexpr static TileID SHALLOW_WATER = 481;
		constexpr static TileID SAND = 484;
		constexpr static TileID STONE = 487;
		constexpr static TileID LIGHT_GRASS = 513;
		constexpr static TileID GRASS = 514;
		constexpr static TileID GRASS_ALT1 = 578;
		constexpr static TileID GRASS_ALT2 = 579;
		constexpr static TileID GRAY = 485;
		constexpr static TileID ROAD = 517;
		constexpr static TileID DIRT = 518;
		constexpr static TileID TILES = 519;
		constexpr static TileID OIL = 580;
		constexpr static TileID IRON_ORE = 581;
		constexpr static TileID DIAMOND_ORE = 582;
		constexpr static TileID COPPER_ORE = 583;
		constexpr static TileID GOLD_ORE = 584;
		constexpr static TileID COAL_ORE = 585;
		constexpr static TileID MARKET1 = 608;
		constexpr static TileID MARKET2 = 609;
		constexpr static TileID MARKET3 = 610;
		constexpr static TileID BLACKSMITH1 = 611;
		constexpr static TileID BLACKSMITH2 = 612;
		constexpr static TileID BLACKSMITH3 = 613;
		constexpr static TileID TOWER_NW = 640;
		constexpr static TileID TOWER_NE = 641;
		constexpr static TileID TOWER_SW = 642;
		constexpr static TileID TOWER_SE = 643;
		constexpr static TileID TOWER_WE = 644;
		constexpr static TileID TOWER_NS = 645;
		constexpr static TileID TOWER_N = 646;
		constexpr static TileID TOWER_S = 647;
		constexpr static TileID HOUSE1 = 672;
		constexpr static TileID HOUSE2 = 673;
		constexpr static TileID HOUSE3 = 674;
		constexpr static TileID TREE1 = 675;
		constexpr static TileID TREE2 = 676;
		constexpr static TileID TREE3 = 677;
		constexpr static TileID TREE0 = 678;
		constexpr static TileID KEEP_NW = 704;
		constexpr static TileID KEEP_NE = 705;
		constexpr static TileID MISSING = 708;
		constexpr static TileID IRON_ORE_REGEN = 709;
		constexpr static TileID DIAMOND_ORE_REGEN = 710;
		constexpr static TileID COPPER_ORE_REGEN = 711;
		constexpr static TileID GOLD_ORE_REGEN = 712;
		constexpr static TileID COAL_ORE_REGEN = 713;
		constexpr static TileID KEEP_SW = 736;
		constexpr static TileID KEEP_SE = 737;

		static std::unordered_set<TileID> landSet;
		static std::unordered_set<TileID> solidSet;

		constexpr static std::array<TileID, 3> BEDS {BED1, BED2, BED3};
		constexpr static std::array<TileID, 3> PLANTS {PLANT1, PLANT2, PLANT3};
		constexpr static std::array<TileID, 2> DOORS {DOOR1, DOOR2};

		bool isLand(TileID tile) const override {
			return landSet.contains(tile);
		}

		bool isWalkable(TileID tile) const override {
			return landSet.contains(tile) || tile == SHALLOW_WATER || tile == EMPTY || tile == OIL || !isSolid(tile);
		}

		bool isSolid(TileID tile) const override {
			return solidSet.contains(tile);
		}

		bool isResource(TileID tile) const {
			return OIL <= tile && tile <= COAL_ORE;
		}

		TileID getEmpty() const override { return EMPTY; }

		const char * name() const override {
			return "Monomap";
		}

		Texture & getTexture() override {
			return cacheTexture("resources/tileset.png");
		}
	};

	extern Monomap monomap;
	extern std::unordered_map<RealmID, std::shared_ptr<TileSet>> tileSets;
}
