#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Texture.h"
#include "Types.h"

namespace Game3 {
	class ItemStack;

	struct TileSet {
		TileSet() = default;
		virtual ~TileSet() = default;
		virtual bool isLand(TileID) const = 0;
		virtual bool isWalkable(TileID id) const { return isLand(id) || !isSolid(id); }
		virtual bool isSolid(TileID) const = 0;
		virtual TileID getEmpty() const { return 0; }
		virtual const char * name() const = 0;
		virtual Texture & getTexture() = 0;
		virtual bool getItemStack(TileID, ItemStack &) const = 0;
	};

	struct Monomap: TileSet {
		constexpr static TileID EMPTY         = 0;
		constexpr static TileID CARPET1_NW    = 1;
		constexpr static TileID CARPET1_N     = 2;
		constexpr static TileID CARPET1_NE    = 3;
		constexpr static TileID CARPET1_W     = 33;
		constexpr static TileID CARPET1_C     = 34;
		constexpr static TileID CARPET1_E     = 35;
		constexpr static TileID CARPET1_SW    = 65;
		constexpr static TileID CARPET1_S     = 66;
		constexpr static TileID CARPET1_SE    = 67;
		constexpr static TileID CARPET2_NW    = 9;
		constexpr static TileID CARPET2_N     = 10;
		constexpr static TileID CARPET2_NE    = 11;
		constexpr static TileID CARPET2_W     = 41;
		constexpr static TileID CARPET2_C     = 42;
		constexpr static TileID CARPET2_E     = 43;
		constexpr static TileID CARPET2_SW    = 73;
		constexpr static TileID CARPET2_S     = 74;
		constexpr static TileID CARPET2_SE    = 75;
		constexpr static TileID CARPET3_NW    = 17;
		constexpr static TileID CARPET3_N     = 18;
		constexpr static TileID CARPET3_NE    = 19;
		constexpr static TileID CARPET3_W     = 49;
		constexpr static TileID CARPET3_C     = 50;
		constexpr static TileID CARPET3_E     = 51;
		constexpr static TileID CARPET3_SW    = 81;
		constexpr static TileID CARPET3_S     = 82;
		constexpr static TileID CARPET3_SE    = 83;
		constexpr static TileID WALL_SE       = 192;
		constexpr static TileID WALL_WE       = 193;
		constexpr static TileID WALL_SW       = 194;
		constexpr static TileID WALL_N        = 195;
		constexpr static TileID WALL_E        = 196;
		constexpr static TileID WALL_WE2      = 197;
		constexpr static TileID WALL_W        = 198;
		constexpr static TileID TOWER_SE      = 199;
		constexpr static TileID TOWER_WE      = 200;
		constexpr static TileID TOWER_SW      = 201;
		constexpr static TileID TOWER_N       = 202;
		constexpr static TileID TOWER_E       = 203;
		constexpr static TileID TOWER_WE2     = 204;
		constexpr static TileID TOWER_W       = 205;
		constexpr static TileID WALL_NS       = 224;
		constexpr static TileID FLOOR         = 225;
		constexpr static TileID WALL_NS2      = 226;
		constexpr static TileID WALL_NS3      = 227;
		constexpr static TileID WALL_WES      = 229;
		constexpr static TileID TOWER_NS      = 231;
		constexpr static TileID TOWER_NS2     = 233;
		constexpr static TileID TOWER_NS3     = 234;
		constexpr static TileID TOWER_WES     = 236;
		constexpr static TileID WALL_NE       = 256;
		constexpr static TileID WALL_WE3      = 257;
		constexpr static TileID WALL_NW       = 258;
		constexpr static TileID WALL_S        = 259;
		constexpr static TileID WALL_NES      = 260;
		constexpr static TileID WALL_NWES     = 261;
		constexpr static TileID WALL_NWS      = 262;
		constexpr static TileID TOWER_NE      = 263;
		constexpr static TileID TOWER_WE3     = 264;
		constexpr static TileID TOWER_NW      = 265;
		constexpr static TileID TOWER_S       = 266;
		constexpr static TileID TOWER_NES     = 267;
		constexpr static TileID TOWER_NWES    = 268;
		constexpr static TileID TOWER_NWS     = 269;
		constexpr static TileID WALL          = 289;
		constexpr static TileID WALL_NWE      = 293;
		constexpr static TileID TOWER         = 296;
		constexpr static TileID TOWER_NWE     = 300;
		constexpr static TileID COUNTER       = 288;
		constexpr static TileID COUNTER_W     = 290;
		constexpr static TileID COUNTER_WE    = 291;
		constexpr static TileID COUNTER_E     = 292;
		constexpr static TileID DOOR1         = 322;
		constexpr static TileID DOOR2         = 324;
		constexpr static TileID STOCKPILE_W   = 326;
		constexpr static TileID STOCKPILE_E   = 327;
		constexpr static TileID TABLE_W       = 330;
		constexpr static TileID TABLE_WE      = 331;
		constexpr static TileID TABLE_E       = 334;
		constexpr static TileID CHAIR_N       = 335;
		constexpr static TileID CHAIR_W       = 337;
		constexpr static TileID CHAIR_E       = 338;
		constexpr static TileID CHAIR_S       = 339;
		constexpr static TileID BED1          = 352;
		constexpr static TileID BED2          = 353;
		constexpr static TileID BED3          = 354;
		constexpr static TileID BOOKSHELF     = 355;
		constexpr static TileID PLANT1        = 388;
		constexpr static TileID PLANT2        = 389;
		constexpr static TileID PLANT3        = 390;
		constexpr static TileID CAULDRON_EMPTY       = 392;
		constexpr static TileID CAULDRON_RED_EMPTY   = 393;
		constexpr static TileID CAULDRON_BLUE_EMPTY  = 394;
		constexpr static TileID CAULDRON_GREEN_EMPTY = 395;
		constexpr static TileID CAULDRON_FULL        = 424;
		constexpr static TileID CAULDRON_RED_FULL    = 425;
		constexpr static TileID CAULDRON_BLUE_FULL   = 426;
		constexpr static TileID CAULDRON_GREEN_FULL  = 427;
		constexpr static TileID ANVIL         = 453;
		constexpr static TileID FURNACE       = 454;
		constexpr static TileID SHALLOW_WATER = 481;
		constexpr static TileID WATER         = 482;
		constexpr static TileID DEEP_WATER    = 483;
		constexpr static TileID SAND          = 484;
		constexpr static TileID GRAY          = 485;
		constexpr static TileID DEEPER_WATER  = 486;
		constexpr static TileID STONE         = 487;
		constexpr static TileID CAVE_WALL     = 488;
		constexpr static TileID CAVE_IRON     = 489;
		constexpr static TileID CAVE_DIAMOND  = 490;
		constexpr static TileID CAVE_COPPER   = 491;
		constexpr static TileID CAVE_GOLD     = 492;
		constexpr static TileID CAVE_COAL     = 493;
		constexpr static TileID LIGHT_GRASS   = 513;
		constexpr static TileID GRASS         = 514;
		constexpr static TileID ROAD          = 517;
		constexpr static TileID DIRT          = 518;
		constexpr static TileID TILES         = 519;
		constexpr static TileID SNOW          = 548;
		constexpr static TileID ASH           = 549;
		constexpr static TileID FOREST_FLOOR  = 552;
		constexpr static TileID GRASS_ALT1    = 578;
		constexpr static TileID GRASS_ALT2    = 579;
		constexpr static TileID OIL           = 580;
		constexpr static TileID IRON_ORE      = 581;
		constexpr static TileID DIAMOND_ORE   = 582;
		constexpr static TileID COPPER_ORE    = 583;
		constexpr static TileID GOLD_ORE      = 584;
		constexpr static TileID COAL_ORE      = 585;
		constexpr static TileID MARKET1       = 608;
		constexpr static TileID MARKET2       = 609;
		constexpr static TileID MARKET3       = 610;
		constexpr static TileID BLACKSMITH1   = 611;
		constexpr static TileID BLACKSMITH2   = 612;
		constexpr static TileID BLACKSMITH3   = 613;
		constexpr static TileID TAVERN1       = 614;
		constexpr static TileID HOUSE1        = 672;
		constexpr static TileID HOUSE2        = 673;
		constexpr static TileID HOUSE3        = 674;
		constexpr static TileID TREE1         = 675;
		constexpr static TileID TREE2         = 676;
		constexpr static TileID TREE3         = 677;
		constexpr static TileID TREE0         = 678;
		constexpr static TileID TREE1_EMPTY   = 679;
		constexpr static TileID TREE2_EMPTY   = 680;
		constexpr static TileID TREE3_EMPTY   = 681;
		constexpr static TileID TREE1_FULL    = 682;
		constexpr static TileID TREE2_FULL    = 683;
		constexpr static TileID TREE3_FULL    = 684;
		constexpr static TileID KEEP_NW       = 704;
		constexpr static TileID KEEP_NE       = 705;
		constexpr static TileID LADDER        = 706;
		constexpr static TileID CAVE          = 707;
		constexpr static TileID MISSING       = 708;
		constexpr static TileID IRON_ORE_REGEN    = 709;
		constexpr static TileID DIAMOND_ORE_REGEN = 710;
		constexpr static TileID COPPER_ORE_REGEN  = 711;
		constexpr static TileID GOLD_ORE_REGEN    = 712;
		constexpr static TileID COAL_ORE_REGEN    = 713;
		constexpr static TileID KEEP_SW       = 736;
		constexpr static TileID KEEP_SE       = 737;
		constexpr static TileID VOID          = 738;
		constexpr static TileID CHARRED_STUMP = 742;

		static std::unordered_set<TileID> landSet;
		static std::unordered_set<TileID> solidSet;
		static std::unordered_set<TileID> woodenWalls;
		static std::unordered_set<TileID> dirtSet;
		static std::unordered_set<TileID> towerSet;

		constexpr static std::array<TileID, 3> BEDS   {BED1,   BED2,   BED3};
		constexpr static std::array<TileID, 3> PLANTS {PLANT1, PLANT2, PLANT3};
		constexpr static std::array<TileID, 2> DOORS  {DOOR1,  DOOR2};

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

		bool getItemStack(TileID, ItemStack &) const override;
	};

	extern Monomap monomap;
	extern std::unordered_map<RealmID, std::shared_ptr<TileSet>> tileSets;
}
