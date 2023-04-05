#include "Tiles.h"
#include "item/Item.h"
#include "realm/Realm.h"

namespace Game3 {
	const std::unordered_set<TileID> Monomap::landSet {
		SAND, LIGHT_GRASS, GRASS, GRASS_ALT1, GRASS_ALT2, ROAD, DIRT, TILES, STONE, FLOOR, ASH, SNOW,
		CARPET1_NW, CARPET1_N, CARPET1_NE, CARPET1_W, CARPET1_C, CARPET1_E, CARPET1_SW, CARPET1_S, CARPET1_SE,
		CARPET2_NW, CARPET2_N, CARPET2_NE, CARPET2_W, CARPET2_C, CARPET2_E, CARPET2_SW, CARPET2_S, CARPET2_SE,
		CARPET3_NW, CARPET3_N, CARPET3_NE, CARPET3_W, CARPET3_C, CARPET3_E, CARPET3_SW, CARPET3_S, CARPET3_SE,
		VOLCANIC_SAND, VOLCANIC_ROCK,
	};

	const std::unordered_set<TileID> Monomap::solidSet {
		TOWER_NW, TOWER_WE, TOWER_NE, TOWER_E, TOWER_W, TOWER_NS, TOWER_SW, TOWER_WE3, TOWER_SE, TOWER_N, TOWER_WE2, TOWER_NS2, TOWER_NS3, TOWER_S, TOWER_NWES, TOWER_NWS, TOWER_NES, TOWER_WES, TOWER,
		TOWER_NWE, HOUSE1, HOUSE2, HOUSE3, MARKET1, MARKET2, MARKET3, KEEP_NW, KEEP_NE, KEEP_SW, KEEP_SE, IRON_ORE, DIAMOND_ORE, COPPER_ORE, GOLD_ORE, COAL_ORE, BLACKSMITH1, BLACKSMITH2, BLACKSMITH3,
		TREE1, TREE2, TREE3, IRON_ORE_REGEN, DIAMOND_ORE_REGEN, COPPER_ORE_REGEN, GOLD_ORE_REGEN, COAL_ORE_REGEN, WALL_NW, WALL_WE, WALL_NE, WALL_NS, WALL_SW, WALL_WE3, WALL_SE, WALL_E, WALL_W,
		BOOKSHELF, PLANT1, PLANT2, PLANT3, STOCKPILE_W, STOCKPILE_E, COUNTER, COUNTER_W, COUNTER_WE, COUNTER_E, ANVIL, FURNACE, WALL_N, WALL_WE2, WALL_NS2, WALL_NS3, WALL_S, WALL_NWES, WALL_NWS,
		WALL_NES, WALL_WES, WALL, WALL_NWE, DEEPER_WATER, DEEP_WATER, WATER, CAVE, VOID, CAVE_WALL, CAVE_IRON, CAVE_DIAMOND, CAVE_COPPER, CAVE_GOLD, CAVE_COAL, TAVERN1, TABLE_W, TABLE_WE, TABLE_E,
		CHARRED_STUMP, TREE1_EMPTY, TREE2_EMPTY, TREE3_EMPTY, TREE1_FULL, TREE2_FULL, TREE3_FULL, LAVA, CAULDRON_EMPTY, CAULDRON_RED_EMPTY, CAULDRON_BLUE_EMPTY, CAULDRON_GREEN_EMPTY, CAULDRON_FULL,
		CAULDRON_RED_FULL, CAULDRON_BLUE_FULL, CAULDRON_GREEN_FULL,
	};

	const std::unordered_set<TileID> Monomap::woodenWalls {
		WALL_NW, WALL_WE, WALL_NE, WALL_E, WALL_W, WALL_NS, WALL_SW, WALL_WE3, WALL_SE, WALL_N, WALL_WE2, WALL_NS2, WALL_NS3, WALL_S, WALL_NWES, WALL_NWS, WALL_NES, WALL_WES, WALL, WALL_NWE,
	};

	const std::unordered_set<TileID> Monomap::dirtSet {
		LIGHT_GRASS, GRASS, GRASS_ALT1, GRASS_ALT2, DIRT,
	};

	const std::unordered_set<TileID> Monomap::towerSet {
		TOWER_NW, TOWER_WE, TOWER_NE, TOWER_E, TOWER_W, TOWER_NS, TOWER_SW, TOWER_WE3, TOWER_SE, TOWER_N, TOWER_WE2, TOWER_NS2, TOWER_NS3, TOWER_S, TOWER_NWES, TOWER_NWS, TOWER_NES, TOWER_WES, TOWER,
		TOWER_NWE,
	};

	const std::unordered_set<TileID> Monomap::oreSpawnSet {
		STONE, VOLCANIC_ROCK,
	};

	bool Monomap::getItemStack(TileID tile, ItemStack &stack) const {
		if (woodenWalls.contains(tile)) {
			stack = {Item::WOODEN_WALL, 1};
			return true;
		}

		if (towerSet.contains(tile)) {
			stack = {Item::TOWER, 1};
			return true;
		}

		switch (tile) {
			case PLANT1: stack = {Item::PLANT_POT1, 1}; break;
			case PLANT2: stack = {Item::PLANT_POT2, 1}; break;
			case PLANT3: stack = {Item::PLANT_POT3, 1}; break;
			default:
				return false;
		}

		return true;
	}

	Monomap monomap;

	// A little vestigial.
	std::unordered_map<RealmType, std::shared_ptr<TileSet>> tileSets {
		{Realm::OVERWORLD,  std::make_shared<Monomap>()},
		{Realm::HOUSE,      std::make_shared<Monomap>()},
		{Realm::KEEP,       std::make_shared<Monomap>()},
		{Realm::BLACKSMITH, std::make_shared<Monomap>()},
		{Realm::CAVE,       std::make_shared<Monomap>()},
		{Realm::TAVERN,     std::make_shared<Monomap>()},
	};
}
