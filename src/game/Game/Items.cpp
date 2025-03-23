#include "entity/Bomb.h"
#include "entity/Egg.h"
#include "entity/Snowball.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/AutocrafterItem.h"
#include "item/AutofarmerItem.h"
#include "item/BasicFood.h"
#include "item/BiomassLiquefierItem.h"
#include "item/Cake.h"
#include "item/CaveEntrance.h"
#include "item/CentrifugeItem.h"
#include "item/ChemicalItem.h"
#include "item/ChemicalReactorItem.h"
#include "item/CombinerItem.h"
#include "item/ComputerItem.h"
#include "item/ContainmentOrb.h"
#include "item/Copier.h"
#include "item/CreativeGeneratorItem.h"
#include "item/DisruptorItem.h"
#include "item/DissolverItem.h"
#include "item/Drink.h"
#include "item/EmptyFlask.h"
#include "item/EntitySpawnItem.h"
#include "item/EternalFountainItem.h"
#include "item/FilledFlask.h"
#include "item/Floor.h"
#include "item/Flower.h"
#include "item/FluidGun.h"
#include "item/Furniture.h"
#include "item/GeothermalGeneratorItem.h"
#include "item/Hammer.h"
#include "item/Hoe.h"
#include "item/IncineratorItem.h"
#include "item/IncubatorItem.h"
#include "item/Item.h"
#include "item/LampItem.h"
#include "item/Landfill.h"
#include "item/LiquefierItem.h"
#include "item/MeleeWeapon.h"
#include "item/MicroscopeItem.h"
#include "item/Mutagen.h"
#include "item/MutatorItem.h"
#include "item/Mushroom.h"
#include "item/Pickaxe.h"
#include "item/PipeItem.h"
#include "item/PressurePlateItem.h"
#include "item/ProjectileItem.h"
#include "item/PumpItem.h"
#include "item/RecombinatorItem.h"
#include "item/Sapling.h"
#include "item/SequencerItem.h"
#include "item/Seed.h"
#include "item/TankItem.h"
#include "item/TerrainSeed.h"
#include "item/Tool.h"
#include "item/VoidFlask.h"
#include "item/VoidPickaxe.h"
#include "tileentity/Chest.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Crate.h"
#include "tileentity/Microscope.h"

namespace Game3 {
	void Game::addItems() {
		add(std::make_shared<AutocrafterItem>("base:item/autocrafter", "Autocrafter", 999, 64)); // TODO: cost

		add(std::make_shared<AutofarmerItem>("base:item/autofarmer", "Autofarmer", 999, 64)); // TODO: cost

		add(std::make_shared<BasicFood>("base:item/tomato",      "Tomato",          8));
		add(std::make_shared<BasicFood>("base:item/corn",        "Corn",            8));
		add(std::make_shared<BasicFood>("base:item/pumpkin",     "Pumpkin",         8));
		add(std::make_shared<BasicFood>("base:item/carrot",      "Carrot",          8));
		add(std::make_shared<BasicFood>("base:item/potato",      "Potato",          8));
		add(std::make_shared<BasicFood>("base:item/strawberry",  "Strawberry",      8));
		add(std::make_shared<BasicFood>("base:item/eggplant",    "Eggplant",        8));
		add(std::make_shared<BasicFood>("base:item/cabbage",     "Cabbage",         8));
		add(std::make_shared<BasicFood>("base:item/onion",       "Onion",           8));
		add(std::make_shared<BasicFood>("base:item/bean",        "Bean",            8));
		add(std::make_shared<BasicFood>("base:item/chili",       "Chili",           9));
		add(std::make_shared<BasicFood>("base:item/cactus",      "Cactus",          4, -1));
		add(std::make_shared<BasicFood>("base:item/baguette",    "Baguette",       24,  8));
		add(std::make_shared<BasicFood>("base:item/bread",       "Bread",          12,  4));
		add(std::make_shared<BasicFood>("base:item/cheese",      "Cheese",         32, 10));
		add(std::make_shared<BasicFood>("base:item/chili_crisp", "Chili Crisp",    32, 10));
		add(std::make_shared<BasicFood>("base:item/morsel",      "Monster Morsel", 10,  2));
		add(std::make_shared<BasicFood>("base:item/raw_meat",    "Raw Meat",       10,  2));
		add(std::make_shared<BasicFood>("base:item/meat",        "Meat",           20,  4));
		add(std::make_shared<BasicFood>("base:item/bleach",      "Bleach",         16, -8));

		add(std::make_shared<BiomassLiquefierItem>("base:item/biomass_liquefier", "Biomass Liquefier", 999, 64)); // TODO: cost

		add(std::make_shared<Cake>("base:item/cake", "Cake", 999, 16)); // TODO: cost

		add(std::make_shared<CaveEntrance>("base:item/cave_entrance", "Cave Entrance", 50, 1));

		add(std::make_shared<CentrifugeItem>("base:item/centrifuge", "Centrifuge", 999, 64)); // TODO: cost

		add(std::make_shared<ChemicalItem>("base:item/chemical", "Chemical", 0));

		add(std::make_shared<ChemicalReactorItem>("base:item/chemical_reactor", "Chemical Reactor", 999, 64)); // TODO: cost

		add(std::make_shared<CombinerItem>("base:item/combiner", "Combiner", 999, 64)); // TODO: cost

#ifdef GAME3_ENABLE_SCRIPTING
		add(std::make_shared<ComputerItem>("base:item/computer", "Computer", 999, 64)); // TODO: cost
#endif

		add(std::make_shared<ContainmentOrb>("base:item/contorb", "Containment Orb", 64, 1)); // TODO: cost

		add(std::make_shared<Copier>("base:item/copier", "Copier", 0, 1));

		add(std::make_shared<CreativeGeneratorItem>("base:item/creative_generator", "Creative Generator", 999, 64)); // TODO: cost

		add(std::make_shared<DataPipeItem>(4));

		add(std::make_shared<DesertSapling>("base:item/desert_sapling", "Cactus Sapling", 5, 64));

		add(std::make_shared<DisruptorItem>("base:item/disruptor", "Disruptor", 999, 64)); // TODO: cost

		add(std::make_shared<DissolverItem>("base:item/dissolver", "Dissolver", 999, 64)); // TODO: cost

		add(std::make_shared<Milk>("base:item/milk_flask", "Milk", 4, 64));

		add(std::make_shared<EmptyFlask>("base:item/flask", "Flask", 2, 64));

		add(std::make_shared<EntitySpawnItem>("base:item/ship", "Ship", 999, "base:entity/ship")); // TODO: cost

		add(std::make_shared<EnergyPipeItem>(4));

		add(std::make_shared<EternalFountainItem>("base:item/eternal_fountain", "Eternal Fountain", 999, 64)); // TODO: cost

		add(std::make_shared<FilledFlask>("base:item/water_flask", "Water Flask", 3, "base:fluid/water"));
		add(std::make_shared<FilledFlask>("base:item/lava_flask",  "Lava Flask",  4, "base:fluid/lava"));
		add(std::make_shared<FilledFlask>("base:item/brine_flask", "Brine Flask", 4, "base:fluid/brine"));
		add(std::make_shared<FilledFlask>("base:item/honey",       "Honey",       5, "base:fluid/honey"));
		add(std::make_shared<FilledFlask>("base:item/liquid_biomass_flask", "Liquid Biomass Flask", 32, "base:fluid/liquid_biomass"));
		add(std::make_shared<FilledFlask>("base:item/mutagen_flask", "Mutagen Flask", 32, "base:fluid/mutagen"));

		add(std::make_shared<Floor>("base:item/floor",       "Floor",       "base:tile/floor",       4, 64));
		add(std::make_shared<Floor>("base:item/stone_tiles", "Stone Tiles", "base:tile/stone_tiles", 4, 64));

		add(std::make_shared<FluidGun>("base:item/fluid_gun", "Fluid Gun", 999, 1)); // TODO: cost

		add(std::make_shared<FluidPipeItem>(4));

		add(std::make_shared<GeothermalGeneratorItem>("base:item/geothermal_generator", "Geothermal Generator", 999, 64)); // TODO: cost

		add(std::make_shared<GrasslandSapling>("base:item/sapling", "Sapling", 5, 64));

		add(std::make_shared<Hammer>("base:item/iron_hammer",    "Iron Hammer",     40,  3.f, 128));
		add(std::make_shared<Hammer>("base:item/gold_hammer",    "Gold Hammer",    100, .75f, 128));
		add(std::make_shared<Hammer>("base:item/diamond_hammer", "Diamond Hammer", 210,  1.f, 128));

		add(std::make_shared<Hoe>("base:item/iron_hoe", "Iron Hoe", 40, 128));

		add(std::make_shared<IncineratorItem>("base:item/incinerator", "Incinerator", 999, 64)); // TODO: cost

		add(std::make_shared<IncubatorItem>("base:item/incubator", "Incubator", 999, 64)); // TODO: cost

		add(std::make_shared<Item>("base:item/shortsword",       "Shortsword",        100,  1));
		add(std::make_shared<Item>("base:item/red_potion",       "Red Potion",         20,  8));
		add(std::make_shared<Item>("base:item/coins",            "Gold",                1, 1'000'000));
		add(std::make_shared<Item>("base:item/iron_ore",         "Iron Ore",           10, 64));
		add(std::make_shared<Item>("base:item/copper_ore",       "Copper Ore",          8, 64));
		add(std::make_shared<Item>("base:item/gold_ore",         "Gold Ore",           20, 64));
		add(std::make_shared<Item>("base:item/diamond_ore",      "Diamond Ore",        80, 64));
		add(std::make_shared<Item>("base:item/uranium_ore",      "Uranium Ore",       100, 64));
		add(std::make_shared<Item>("base:item/diamond",          "Diamond",           100, 64));
		add(std::make_shared<Item>("base:item/coal",             "Coal",                5, 64));
		add(std::make_shared<Item>("base:item/oil",              "Oil",                15, 64));
		add(std::make_shared<Item>("base:item/wood",             "Wood",                3, 64));
		add(std::make_shared<Item>("base:item/iron_bar",         "Iron Bar",           16, 64));
		add(std::make_shared<Item>("base:item/gold_bar",         "Gold Bar",           45, 64));
		add(std::make_shared<Item>("base:item/copper_bar",       "Copper Bar",         10, 64));
		add(std::make_shared<Item>("base:item/plank",            "Plank",               4, 64));
		add(std::make_shared<Item>("base:item/brick",            "Brick",               3, 64));
		add(std::make_shared<Item>("base:item/pot",              "Pot",                24, 64));
		add(std::make_shared<Item>("base:item/ash",              "Ash",                 1, 64));
		add(std::make_shared<Item>("base:item/silicon",          "Silicon",             2, 64));
		add(std::make_shared<Item>("base:item/electronics",      "Electronics",        32, 64));
		add(std::make_shared<Item>("base:item/sulfur",           "Sulfur",             15, 64));
		add(std::make_shared<Item>("base:item/cotton",           "Cotton",              8, 64));
		add(std::make_shared<Item>("base:item/wheat",            "Wheat",               8, 64));
		add(std::make_shared<Item>("base:item/red_dye",          "Red Dye",            12, 64));
		add(std::make_shared<Item>("base:item/orange_dye",       "Orange Dye",         12, 64));
		add(std::make_shared<Item>("base:item/yellow_dye",       "Yellow Dye",         12, 64));
		add(std::make_shared<Item>("base:item/green_dye",        "Green Dye",          12, 64));
		add(std::make_shared<Item>("base:item/blue_dye",         "Blue Dye",           12, 64));
		add(std::make_shared<Item>("base:item/purple_dye",       "Purple Dye",         12, 64));
		add(std::make_shared<Item>("base:item/white_dye",        "White Dye",          12, 64));
		add(std::make_shared<Item>("base:item/black_dye",        "Black Dye",          12, 64));
		add(std::make_shared<Item>("base:item/brown_dye",        "Brown Dye",          12, 64));
		add(std::make_shared<Item>("base:item/pink_dye",         "Pink Dye",           12, 64));
		add(std::make_shared<Item>("base:item/light_blue_dye",   "Light Blue Dye",     12, 64));
		add(std::make_shared<Item>("base:item/gray_dye",         "Gray Dye",           12, 64));
		add(std::make_shared<Item>("base:item/lime_dye",         "Lime Dye",           12, 64));
		add(std::make_shared<Item>("base:item/saffron_milkcap",  "Saffron Milkcap",     8, 64));
		add(std::make_shared<Item>("base:item/honey_fungus",     "Honey Fungus",        8, 64));
		add(std::make_shared<Item>("base:item/brittlegill",      "Golden Brittlegill", 10, 64));
		add(std::make_shared<Item>("base:item/indigo_milkcap",   "Indigo Milkcap",     10, 64));
		add(std::make_shared<Item>("base:item/black_trumpet",    "Black Trumpet",      10, 64));
		add(std::make_shared<Item>("base:item/grey_knight",      "Grey Knight",        10, 64));
		add(std::make_shared<Item>("base:item/fire_opal",        "Fire Opal",          50, 64));
		add(std::make_shared<Item>("base:item/flour",            "Flour",              10, 64));
		add(std::make_shared<Item>("base:item/dough",            "Dough",              12, 64));
		add(std::make_shared<Item>("base:item/knife",            "Knife",              20, 64));
		add(std::make_shared<Item>("base:item/yeast",            "Yeast",               4, 64));
		add(std::make_shared<Item>("base:item/clay",             "Clay",                2, 64));
		add(std::make_shared<Item>("base:item/glass",            "Glass",               2, 64));
		add(std::make_shared<Item>("base:item/salt",             "Salt",                4, 64));
		add(std::make_shared<Item>("base:item/sugar",            "Sugar",               5, 64));
		add(std::make_shared<Item>("base:item/protein",          "Protein",            12, 64));
		add(std::make_shared<Item>("base:item/eye",              "Eye",                24, 64));
		add(std::make_shared<Item>("base:item/chromatin",        "Chromatin",           2, 64));
		add(std::make_shared<Item>("base:item/fabric",           "Fabric",             20, 64));
		add(std::make_shared<Item>("base:item/sulfuric_acid",    "Sulfuric Acid",      16, 64));
		add(std::make_shared<Item>("base:item/gene",             "Gene",                1,  1));
		add(std::make_shared<Item>("base:item/genetic_template", "Genetic Template",    1,  1));
		add(std::make_shared<Item>("base:item/aluminum_ore",     "Aluminum Ore",       32, 64));
		add(std::make_shared<Item>("base:item/apatite_ore",      "Apatite Ore",        32, 64));
		add(std::make_shared<Item>("base:item/asbestos_ore",     "Asbestos Ore",       32, 64));
		add(std::make_shared<Item>("base:item/bauxite_ore",      "Bauxite Ore",        32, 64));
		add(std::make_shared<Item>("base:item/bornite_ore",      "Bornite Ore",        32, 64));
		add(std::make_shared<Item>("base:item/cassiterite_ore",  "Cassiterite Ore",    32, 64));
		add(std::make_shared<Item>("base:item/chalcopyrite_ore", "Chalcopyrite Ore",   32, 64));
		add(std::make_shared<Item>("base:item/chromite_ore",     "Chromite Ore",       32, 64));
		add(std::make_shared<Item>("base:item/chrysoberyl_ore",  "Chrysoberyl Ore",    32, 64));
		add(std::make_shared<Item>("base:item/garnet_ore",       "Garnet Ore",         32, 64));
		add(std::make_shared<Item>("base:item/ilmenite_ore",     "Ilmenite Ore",       32, 64));
		add(std::make_shared<Item>("base:item/magnetite_ore",    "Magnetite Ore",      32, 64));
		add(std::make_shared<Item>("base:item/malachite_ore",    "Malachite Ore",      32, 64));
		add(std::make_shared<Item>("base:item/mica_ore",         "Mica Ore",           32, 64));
		add(std::make_shared<Item>("base:item/olivine_ore",      "Olivine Ore",        32, 64));
		add(std::make_shared<Item>("base:item/pyrite_ore",       "Pyrite Ore",         32, 64));
		add(std::make_shared<Item>("base:item/pyrochlore_ore",   "Pyrochlore Ore",     32, 64));
		add(std::make_shared<Item>("base:item/pyrope_ore",       "Pyrope Ore",         32, 64));
		add(std::make_shared<Item>("base:item/sodalite_ore",     "Sodalite Ore",       32, 64));
		add(std::make_shared<Item>("base:item/stibnite_ore",     "Stibnite Ore",       32, 64));
		add(std::make_shared<Item>("base:item/sulfur_ore",       "Sulfur Ore",         32, 64));
		add(std::make_shared<Item>("base:item/tetrahedrite_ore", "Tetrahedrite Ore",   32, 64));
		add(std::make_shared<Item>("base:item/vanadium_magnetite_ore", "Vanadium Magnetite Ore", 32, 64));
		add(std::make_shared<Item>("base:item/niobium_bar",      "Niobium Bar",       128, 64));
		add(std::make_shared<Item>("base:item/teleportation_focus", "Teleportation Focus", 999, 64)); // TODO: cost
		add(std::make_shared<Item>("base:item/ticket",           "Ticket",              0, -1));
		add(std::make_shared<Item>("base:item/pickle",           "Pickle",             10, 64));
		add(std::make_shared<Item>("base:item/fire", "Fire", 999, 64)); // TODO: cost
		add(std::make_shared<Item>("base:item/bing", "Bing", 999, 64)); // TODO: cost

		add(std::make_shared<ItemPipeItem>(4));

		add(std::make_shared<LampItem>("base:item/lamp", "Lamp", 999, 64)); // TODO: cost

		add(std::make_shared<Landfill>("base:item/sand",          "Sand",          1, 64, "base:tile/sand"));
		add(std::make_shared<Landfill>("base:item/volcanic_sand", "Volcanic Sand", 3, 64, "base:tile/volcanic_sand"));
		add(std::make_shared<Landfill>("base:item/stone",         "Stone",         1, 64, "base:tile/stone", "base:tile/cave_wall"));
		add(std::make_shared<Landfill>("base:item/grimstone",     "Grimstone",     2, 64, "base:tile/grimstone"));
		add(std::make_shared<Landfill>("base:item/dirt",          "Dirt",          1, 64, "base:tile/dirt"));

		add(std::make_shared<LiquefierItem>("base:item/liquefier", "Liquefier", 999, 64)); // TODO: cost

		add(std::make_shared<Mead>("base:item/mead", "Mead", 10, 16));

		add(std::make_shared<MeleeWeapon>("base:item/iron_sword",    "Iron Sword",    150, 3, 1, 128));
		add(std::make_shared<MeleeWeapon>("base:item/gold_sword",    "Gold Sword",    400, 8, 3,  64));
		add(std::make_shared<MeleeWeapon>("base:item/diamond_sword", "Diamond Sword", 900, 6, 2, 256));
		add(std::make_shared<MeleeWeapon>("base:item/copper_sword",  "Copper Sword",   32, 6, 2, 256));

		add(std::make_shared<MicroscopeItem>("base:item/microscope", "Microscope", 999, 64)); // TODO: cost

		add(std::make_shared<Mutagen>("base:item/mutagen", "Mutagen", 999, "base:fluid/mutagen", 64)); // TODO: cost

		add(std::make_shared<MutatorItem>("base:item/mutator", "Mutator", 999, 64)); // TODO: cost

		add(std::make_shared<Pickaxe>("base:item/iron_pickaxe",    "Iron Pickaxe",      40,  3.f,  64));
		add(std::make_shared<Pickaxe>("base:item/gold_pickaxe",    "Gold Pickaxe",     100, .75f,  64));
		add(std::make_shared<Pickaxe>("base:item/diamond_pickaxe", "Diamond Pickaxe",  210,  1.f, 512));

		add(std::make_shared<PressurePlateItem>("base:item/pressure_plate", "Pressure Plate", 999, 64)); // TODO: cost

		add(std::make_shared<ProjectileItem<Egg>>("base:item/egg", "Egg", 16, 64));
		add(std::make_shared<ProjectileItem<Snowball>>("base:item/snowball", "Snowball", 999, 64)); // TODO: cost
		add(std::make_shared<ProjectileItem<Bomb>>("base:item/bomb", "Bomb", 32, 64));

		add(std::make_shared<PumpItem>("base:item/pump", "Pump", 999, 64)); // TODO: cost

		add(std::make_shared<RecombinatorItem>("base:item/recombinator", "Recombinator", 999, 64)); // TODO: cost

		add(std::make_shared<Seed>("base:item/cotton_seeds",     "Cotton Seeds",      "base:tile/cotton_0",     4));
		add(std::make_shared<Seed>("base:item/wheat_seeds",      "Wheat Seeds",       "base:tile/wheat_0",      4));
		add(std::make_shared<Seed>("base:item/tomato_seeds",     "Tomato Seeds",      "base:tile/tomato_0",     4));
		add(std::make_shared<Seed>("base:item/corn_seeds",       "Corn Seeds",        "base:tile/corn_0",       4));
		add(std::make_shared<Seed>("base:item/pumpkin_seeds",    "Pumpkin Seeds",     "base:tile/pumpkin_0",    4));
		add(std::make_shared<Seed>("base:item/carrot_seeds",     "Carrot Seeds",      "base:tile/carrot_0",     4));
		add(std::make_shared<Seed>("base:item/potato_seeds",     "Potato Seeds",      "base:tile/potato_0",     4));
		add(std::make_shared<Seed>("base:item/strawberry_seeds", "Strawberry Seeds",  "base:tile/strawberry_0", 4));
		add(std::make_shared<Seed>("base:item/eggplant_seeds",   "Eggplant Seeds",    "base:tile/eggplant_0",   4));
		add(std::make_shared<Seed>("base:item/cabbage_seeds",    "Cabbage Seeds",     "base:tile/cabbage_0",    4));
		add(std::make_shared<Seed>("base:item/onion_seeds",      "Onion Seeds",       "base:tile/onion_0",      4));
		add(std::make_shared<Seed>("base:item/bean_seeds",       "Bean Seeds",        "base:tile/bean_0",       4));
		add(std::make_shared<Seed>("base:item/chili_seeds",      "Chili Seeds",       "base:tile/chili_0",      5));

		add(std::make_shared<SequencerItem>("base:item/sequencer", "Sequencer", 999, 64)); // TODO: cost

		add(std::make_shared<SnowySapling>("base:item/snowy_sapling", "Snowy Sapling", 5, 64));

		add(std::make_shared<TankItem>("base:item/tank", "Tank", 999, 64)); // TODO: cost

		add(std::make_shared<TerrainSeed>("base:item/moss", "Moss", "base:tile/dirt", "base:tile/forest_floor", 2));

		add(std::make_shared<Tool>("base:item/iron_shovel",    "Iron Shovel",     20,  3.f,  64, "base:attribute/shovel"));
		add(std::make_shared<Tool>("base:item/iron_axe",       "Iron Axe",        40,  3.f, 128, "base:attribute/axe"));
		add(std::make_shared<Tool>("base:item/gold_shovel",    "Gold Shovel",     50, .75f, 512, "base:attribute/shovel"));
		add(std::make_shared<Tool>("base:item/gold_axe",       "Gold Axe",       100, .75f,  64, "base:attribute/axe"));
		add(std::make_shared<Tool>("base:item/diamond_shovel", "Diamond Shovel", 110,  1.f, 512, "base:attribute/shovel"));
		add(std::make_shared<Tool>("base:item/diamond_axe",    "Diamond Axe",    210,  1.f, 512, "base:attribute/axe"));
		add(std::make_shared<Tool>("base:item/wrench",         "Wrench",          72,  0.f,  -1, "base:attribute/wrench"));

		add(std::make_shared<VoidFlask>("base:item/void_flask", "Void Flask", 128, 1));

		add(std::make_shared<VoidPickaxe>("base:item/void_pickaxe", "Void Pickaxe", 1000, 0.f, -1, "base:attribute/void_pickaxe"));

		add(Furniture::createSimple("base:item/pride_flag",       "Pride Flag",        80, Layer::Highest,   "base:tile/pride_flag"));
		add(Furniture::createSimple("base:item/ace_flag",         "Asexual Flag",      80, Layer::Highest,   "base:tile/ace_flag"));
		add(Furniture::createSimple("base:item/nb_flag",          "Nonbinary Flag",    80, Layer::Highest,   "base:tile/nb_flag"));
		add(Furniture::createSimple("base:item/knives",           "Knives",            10, Layer::Highest,   "base:tile/knives"));
		add(Furniture::createSimple("base:item/planks",           "Planks",           999, Layer::Terrain,   "base:tile/planks")); // TODO: cost
		add(Furniture::createSimple("base:item/ship_wheel",       "Ship Wheel",       999, Layer::Objects,   "base:tile/ship_wheel")); // TODO: cost
		add(Furniture::createSimple("base:item/kitchen_utensils", "Kitchen Utensils",  10, Layer::Highest,   "base:tile/kitchen_utensils"));
		add(Furniture::createSimple("base:item/plant_pot1",       "Plant Pot",         32, Layer::Submerged, "base:tile/plant1"));
		add(Furniture::createSimple("base:item/plant_pot2",       "Plant Pot",         32, Layer::Submerged, "base:tile/plant2"));
		add(Furniture::createSimple("base:item/plant_pot3",       "Plant Pot",         32, Layer::Submerged, "base:tile/plant3"));
		add(Furniture::createSimple("base:item/electric_guitar",  "Electric Guitar",  100, Layer::Objects,   "base:tile/electric_guitar"));
		add(Furniture::createSimple("base:item/red_planter",      "Red Planter",        8, Layer::Highest,   "base:tile/red_planter"));
		add(Furniture::createSimple("base:item/blue_planter",     "Blue Planter",       8, Layer::Highest,   "base:tile/blue_planter"));
		add(Furniture::createSimple("base:item/red_carpet",       "Red Carpet",        24, Layer::Submerged, "base:tile/red_carpet"));
		add(Furniture::createSimple("base:item/blue_carpet",      "Blue Carpet",       24, Layer::Submerged, "base:tile/blue_carpet"));
		add(Furniture::createSimple("base:item/purple_carpet",    "Purple Carpet",     24, Layer::Submerged, "base:tile/purple_carpet"));
		add(Furniture::createSimple("base:item/torch",            "Torch",              2, Layer::Objects,   "base:tile/torch"));
		add(Furniture::createSimple("base:item/blue_bed",         "Blue Bed",          90, Layer::Objects,   "base:tile/bed1"));
		add(Furniture::createSimple("base:item/red_bed",          "Red Bed",           90, Layer::Objects,   "base:tile/bed2"));
		add(Furniture::createSimple("base:item/purple_bed",       "Purple Bed",        90, Layer::Objects,   "base:tile/bed3"));
		add(Furniture::createSimple("base:item/fence_gate",       "Fence Gate ",       10, Layer::Submerged, "base:tile/gate_horizontal"));

		add(Furniture::createMarchable("base:item/wooden_wall",      "Wooden Wall",       9, Layer::Objects,   "base:tile/wooden_wall",      "base:autotile/wooden_walls"));
		add(Furniture::createMarchable("base:item/concrete_wall",    "Concrete Wall",     9, Layer::Objects,   "base:tile/concrete_wall",    "base:autotile/concrete_walls"));
		add(Furniture::createMarchable("base:item/tall_wooden_wall", "Tall Wooden Wall",  9, Layer::Objects,   "base:tile/tall_wooden_wall", "base:autotile/tall_wooden_walls"));
		add(Furniture::createMarchable("base:item/tower",            "Tower",            10, Layer::Objects,   "base:tile/tower",            "base:autotile/towers"));
		add(Furniture::createMarchable("base:item/kitchen_counter",  "Kitchen Counter",  10, Layer::Objects,   "base:tile/kitchen_counter",  "base:autotile/kitchen_counters"));
		add(Furniture::createMarchable("base:item/dining_table",     "Dining Table",     10, Layer::Objects,   "base:tile/dining_table",     "base:autotile/dining_tables"));
		add(Furniture::createMarchable("base:item/fence",            "Fence",            10, Layer::Submerged, "base:tile/fence",            "base:autotile/fences"));

		add(Furniture::createStation("base:item/furnace",       "Furnace",        12, "base:tile/furnace",           "base:station/furnace"));
		add(Furniture::createStation("base:item/anvil",         "Anvil",         150, "base:tile/anvil",             "base:station/anvil"));
		add(Furniture::createStation("base:item/cauldron",      "Cauldron",      175, "base:tile/cauldron_red_full", "base:station/cauldron"));
		add(Furniture::createStation("base:item/purifier",      "Purifier",      300, "base:tile/purifier",          "base:station/purifier"));
		add(Furniture::createStation("base:item/millstone",     "Millstone",      20, "base:tile/millstone",         "base:station/millstone"));
		add(Furniture::createStation("base:item/cutting_board", "Cutting Board",  30, "base:tile/cutting_board",     "base:station/cutting_board"));
		add(Furniture::createStation("base:item/sink",          "Sink",           30, "base:tile/sink",              "base:station/sink"));
		add(Furniture::createStation("base:item/oven",          "Oven",          200, "base:tile/oven",              "base:station/oven"));

		add(Furniture::createTileEntity("base:item/chest", "Chest", 100, [](const Place &place) -> bool {
			auto out = TileEntity::spawn<Chest>(place, "base:tile/chest", place.position, "Chest", "base:item/chest");
			if (!out)
				return false;
			out->setInventory(30);
			return true;
		}));

		add(Furniture::createTileEntity("base:item/crate", "Crate", 999, [](const Place &place) -> bool { // TODO: cost
			auto out = TileEntity::spawn<Crate>(place, "base:tile/crate", place.position, "base:item/crate", "Crate");
			if (!out)
				return false;
			out->setInventory(1);
			return true;
		}));

		add(Furniture::createTileEntity("base:item/barrel", "Barrel", 999, [](const Place &place) -> bool { // TODO: cost
			auto out = TileEntity::spawn<Crate>(place, "base:tile/barrel", place.position, "base:item/barrel", "Barrel");
			if (!out)
				return false;
			out->setInventory(1);
			return true;
		}));

		add(Furniture::createTileEntity("base:item/fridge", "Fridge", 150, [](const Place &place) -> bool {
			auto out = TileEntity::spawn<Chest>(place, "base:tile/fridge", place.position, "Fridge", "base:item/fridge");
			if (!out)
				return false;
			out->setInventory(30);
			return true;
		}));

		add(Furniture::createTileEntity("base:item/cabinet", "Cabinet", 36, [](const Place &place) -> bool {
			auto out = TileEntity::spawn<Chest>(place, "base:tile/cabinet", place.position, "Fridge", "base:item/cabinet");
			if (!out)
				return false;
			out->setInventory(15);
			return true;
		}));

		for (int i = 1; i <= 5; ++i) {
			for (const char *color: {"red", "orange", "yellow", "green", "blue", "purple", "white", "black"}) {
				std::string name = std::string(color) + " Flower";
				name[0] = std::toupper(name[0]);
				std::string post = std::format("flower{}_{}", i, color);
				auto flower = std::make_shared<Flower>(Identifier("base:item/" + post), std::move(name), Identifier("base:tile/" + post), "base:category/plant_soil", 10);
				flower->addAttribute(Identifier("base:attribute/flower_" + std::string(color)));
				add(flower);
			}
		}
	}

	ItemPtr Game::getItem(const ItemID &id) const {
		return itemRegistry->maybe(id);
	}
}
