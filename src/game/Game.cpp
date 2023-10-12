#include <fstream>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "graphics/Tileset.h"
#include "command/local/ChemicalCommand.h"
#include "command/local/LocalCommandFactory.h"
#include "command/local/LoginCommand.h"
#include "command/local/PlayersCommand.h"
#include "command/local/RegisterCommand.h"
#include "command/local/SegfaultCommand.h"
#include "command/local/UsageCommand.h"
#include "entity/Blacksmith.h"
#include "entity/Chicken.h"
#include "entity/Dog.h"
#include "entity/EntityFactory.h"
#include "entity/ItemEntity.h"
#include "entity/Merchant.h"
#include "entity/Miner.h"
#include "entity/Pig.h"
#include "entity/Player.h"
#include "entity/Sheep.h"
#include "entity/Woodcutter.h"
#include "entity/Worker.h"
#include "game/ClientGame.h"
#include "game/Crop.h"
#include "game/Fluids.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "graph/Graph.h"
#include "graphics/ItemTexture.h"
#include "item/Bomb.h"
#include "item/CaveEntrance.h"
#include "item/CentrifugeItem.h"
#include "item/ChemicalItem.h"
#include "item/ChemicalReactorItem.h"
#include "item/ContainmentOrb.h"
#include "item/EmptyFlask.h"
#include "item/FilledFlask.h"
#include "item/Floor.h"
#include "item/Furniture.h"
#include "item/GeothermalGeneratorItem.h"
#include "item/Hammer.h"
#include "item/Hoe.h"
#include "item/Item.h"
#include "item/Landfill.h"
#include "item/Mead.h"
#include "item/Mushroom.h"
#include "item/Pickaxe.h"
#include "item/PipeItem.h"
#include "item/Plantable.h"
#include "item/PumpItem.h"
#include "item/Sapling.h"
#include "item/Seed.h"
#include "item/TankItem.h"
#include "item/Tool.h"
#include "item/VoidFlask.h"
#include "item/VoidPickaxe.h"
#include "packet/PacketFactory.h"
#include "packet/ProtocolVersionPacket.h"
#include "packet/TileEntityPacket.h"
#include "packet/ChunkRequestPacket.h"
#include "packet/TileUpdatePacket.h"
#include "packet/CommandResultPacket.h"
#include "packet/CommandPacket.h"
#include "packet/SelfTeleportedPacket.h"
#include "packet/ChunkTilesPacket.h"
#include "packet/RealmNoticePacket.h"
#include "packet/LoginPacket.h"
#include "packet/LoginStatusPacket.h"
#include "packet/RegisterPlayerPacket.h"
#include "packet/RegistrationStatusPacket.h"
#include "packet/EntityPacket.h"
#include "packet/MovePlayerPacket.h"
#include "packet/ErrorPacket.h"
#include "packet/EntityMovedPacket.h"
#include "packet/SendChatMessagePacket.h"
#include "packet/EntitySetPathPacket.h"
#include "packet/TeleportSelfPacket.h"
#include "packet/InteractPacket.h"
#include "packet/InventorySlotUpdatePacket.h"
#include "packet/DestroyEntityPacket.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/ActiveSlotSetPacket.h"
#include "packet/DestroyTileEntityPacket.h"
#include "packet/ClickPacket.h"
#include "packet/TimePacket.h"
#include "packet/CraftPacket.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/FluidUpdatePacket.h"
#include "packet/HeldItemSetPacket.h"
#include "packet/SetHeldItemPacket.h"
#include "packet/EntityRequestPacket.h"
#include "packet/TileEntityRequestPacket.h"
#include "packet/JumpPacket.h"
#include "packet/DropItemPacket.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/SwapSlotsPacket.h"
#include "packet/MoveSlotsPacket.h"
#include "packet/AgentMessagePacket.h"
#include "packet/SetTileEntityEnergyPacket.h"
#include "packet/SetPlayerStationTypesPacket.h"
#include "packet/EntityChangingRealmsPacket.h"
#include "packet/ChatMessageSentPacket.h"
#include "packet/OpenTextTabPacket.h"
#include "packet/DragPacket.h"
#include "realm/Cave.h"
#include "realm/House.h"
#include "realm/Keep.h"
#include "realm/Overworld.h"
#include "realm/RealmFactory.h"
#include "realm/ShadowRealm.h"
#include "recipe/CentrifugeRecipe.h"
#include "recipe/CraftingRecipe.h"
#include "recipe/GeothermalRecipe.h"
#include "registry/Registries.h"
#include "tile/CaveTile.h"
#include "tile/CropTile.h"
#include "tile/FarmlandTile.h"
#include "tile/ForestFloorTile.h"
#include "tile/GrassTile.h"
#include "tile/Tile.h"
#include "tile/TreeTile.h"
#include "tileentity/Building.h"
#include "tileentity/Centrifuge.h"
#include "tileentity/ChemicalReactor.h"
#include "tileentity/Chest.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/GeothermalGenerator.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Pipe.h"
#include "tileentity/Pump.h"
#include "tileentity/Sign.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Tank.h"
#include "tileentity/Teleporter.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "tools/ItemStitcher.h"
#include "tools/TileStitcher.h"
#include "ui/module/ChemicalReactorModule.h"
#include "ui/module/ExternalInventoryModule.h"
#include "ui/module/EnergyLevelModule.h"
#include "ui/module/FluidLevelsModule.h"
#include "ui/module/ModuleFactory.h"
#include "util/AStar.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	Game::~Game() {
		INFO("\e[31m~Game\e[39m(" << this << ')');
		dying = true;
	}

	bool Game::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1e9;
		time = time + delta;
		++currentTick;
		return true;
	}

	void Game::initRegistries() {
		registries.clear();
		registries.add<CraftingRecipeRegistry>();
		registries.add<ItemRegistry>();
		registries.add<ItemTextureRegistry>();
		registries.add<TextureRegistry>();
		registries.add<EntityTextureRegistry>();
		registries.add<EntityFactoryRegistry>();
		registries.add<TilesetRegistry>();
		registries.add<TileEntityFactoryRegistry>();
		registries.add<OreRegistry>();
		registries.add<RealmFactoryRegistry>();
		registries.add<RealmTypeRegistry>();
		registries.add<RealmDetailsRegistry>();
		registries.add<PacketFactoryRegistry>();
		registries.add<LocalCommandFactoryRegistry>();
		registries.add<FluidRegistry>();
		registries.add<TileRegistry>();
		registries.add<CropRegistry>();
		registries.add<CentrifugeRecipeRegistry>();
		registries.add<GeothermalRecipeRegistry>();
		registries.add<ModuleFactoryRegistry>();
		registries.add<ItemSetRegistry>();
	}

	void Game::addItems() {
		add(std::make_shared<Hoe>("base:item/iron_hoe", "Iron Hoe", 85, 128));

		add(std::make_shared<Bomb>("base:item/bomb", "Bomb", 32, 64));

		add(std::make_shared<Item>("base:item/shortsword",      "Shortsword",        100,  1));
		add(std::make_shared<Item>("base:item/red_potion",      "Red Potion",         20,  8));
		add(std::make_shared<Item>("base:item/coins",           "Gold",                1, 1'000'000));
		add(std::make_shared<Item>("base:item/iron_ore",        "Iron Ore",           10, 64));
		add(std::make_shared<Item>("base:item/copper_ore",      "Copper Ore",          8, 64));
		add(std::make_shared<Item>("base:item/gold_ore",        "Gold Ore",           20, 64));
		add(std::make_shared<Item>("base:item/diamond_ore",     "Diamond Ore",        80, 64));
		add(std::make_shared<Item>("base:item/uranium_ore",     "Uranium Ore",       100, 64));
		add(std::make_shared<Item>("base:item/diamond",         "Diamond",           100, 64));
		add(std::make_shared<Item>("base:item/coal",            "Coal",                5, 64));
		add(std::make_shared<Item>("base:item/oil",             "Oil",                15, 64));
		add(std::make_shared<Item>("base:item/wood",            "Wood",                3, 64));
		add(std::make_shared<Item>("base:item/cactus",          "Cactus",              4, 64));
		add(std::make_shared<Item>("base:item/iron_bar",        "Iron Bar",           16, 64));
		add(std::make_shared<Item>("base:item/gold_bar",        "Gold Bar",           45, 64));
		add(std::make_shared<Item>("base:item/plank",           "Plank",               4, 64));
		add(std::make_shared<Item>("base:item/dirt",            "Dirt",                1, 64));
		add(std::make_shared<Item>("base:item/brick",           "Brick",               3, 64));
		add(std::make_shared<Item>("base:item/pot",             "Pot",                24, 64));
		add(std::make_shared<Item>("base:item/honey",           "Honey",               5, 64));
		add(std::make_shared<Item>("base:item/ash",             "Ash",                 1, 64));
		add(std::make_shared<Item>("base:item/silicon",         "Silicon",             2, 64));
		add(std::make_shared<Item>("base:item/electronics",     "Electronics",        32, 64));
		add(std::make_shared<Item>("base:item/sulfur",          "Sulfur",             15, 64));
		add(std::make_shared<Item>("base:item/cotton",          "Cotton",              8, 64));
		add(std::make_shared<Item>("base:item/wheat",           "Wheat",               8, 64));
		add(std::make_shared<Item>("base:item/tomato",          "Tomato",              8, 64));
		add(std::make_shared<Item>("base:item/corn",            "Corn",                8, 64));
		add(std::make_shared<Item>("base:item/pumpkin",         "Pumpkin",             8, 64));
		add(std::make_shared<Item>("base:item/carrot",          "Carrot",              8, 64));
		add(std::make_shared<Item>("base:item/potato",          "Potato",              8, 64));
		add(std::make_shared<Item>("base:item/strawberry",      "Strawberry",          8, 64));
		add(std::make_shared<Item>("base:item/eggplant",        "Eggplant",            8, 64));
		add(std::make_shared<Item>("base:item/cabbage",         "Cabbage",             8, 64));
		add(std::make_shared<Item>("base:item/onion",           "Onion",               8, 64));
		add(std::make_shared<Item>("base:item/bean",            "Bean",                8, 64));
		add(std::make_shared<Item>("base:item/red_dye",         "Red Dye",            12, 64));
		add(std::make_shared<Item>("base:item/orange_dye",      "Orange Dye",         12, 64));
		add(std::make_shared<Item>("base:item/yellow_dye",      "Yellow Dye",         12, 64));
		add(std::make_shared<Item>("base:item/green_dye",       "Green Dye",          12, 64));
		add(std::make_shared<Item>("base:item/blue_dye",        "Blue Dye",           12, 64));
		add(std::make_shared<Item>("base:item/purple_dye",      "Purple Dye",         12, 64));
		add(std::make_shared<Item>("base:item/white_dye",       "White Dye",          12, 64));
		add(std::make_shared<Item>("base:item/black_dye",       "Black Dye",          12, 64));
		add(std::make_shared<Item>("base:item/brown_dye",       "Brown Dye",          12, 64));
		add(std::make_shared<Item>("base:item/pink_dye",        "Pink Dye",           12, 64));
		add(std::make_shared<Item>("base:item/light_blue_dye",  "Light Blue Dye",     12, 64));
		add(std::make_shared<Item>("base:item/gray_dye",        "Gray Dye",           12, 64));
		add(std::make_shared<Item>("base:item/lime_dye",        "Lime Dye",           12, 64));
		add(std::make_shared<Item>("base:item/saffron_milkcap", "Saffron Milkcap",     8, 64));
		add(std::make_shared<Item>("base:item/honey_fungus",    "Honey Fungus",        8, 64));
		add(std::make_shared<Item>("base:item/brittlegill",     "Golden Brittlegill", 10, 64));
		add(std::make_shared<Item>("base:item/indigo_milkcap",  "Indigo Milkcap",     10, 64));
		add(std::make_shared<Item>("base:item/black_trumpet",   "Black Trumpet",      10, 64));
		add(std::make_shared<Item>("base:item/grey_knight",     "Grey Knight",        10, 64));
		add(std::make_shared<Item>("base:item/fire_opal",       "Fire Opal",          50, 64));
		add(std::make_shared<Item>("base:item/flour",           "Flour",              10, 64));
		add(std::make_shared<Item>("base:item/dough",           "Dough",              12, 64));
		add(std::make_shared<Item>("base:item/baguette",        "Baguette",           12, 64));
		add(std::make_shared<Item>("base:item/cheese",          "Cheese",              8, 64));
		add(std::make_shared<Item>("base:item/knife",           "Knife",              20, 64));
		add(std::make_shared<Item>("base:item/bread",           "Bread",               6, 64));
		add(std::make_shared<Item>("base:item/yeast",           "Yeast",               4, 64));
		add(std::make_shared<Item>("base:item/clay",            "Clay",                2, 64));
		add(std::make_shared<Item>("base:item/egg",             "Egg",                16, 64));
		add(std::make_shared<Item>("base:item/glass",           "Glass",               2, 64));

		add(std::make_shared<Mead>("base:item/mead", "Mead", 10, 16));

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

		add(std::make_shared<Tool>("base:item/iron_shovel",     "Iron Shovel",      120,  3.f,  64, "base:attribute/shovel"));
		add(std::make_shared<Tool>("base:item/iron_axe",        "Iron Axe",         150,  3.f, 128, "base:attribute/axe"));
		add(std::make_shared<Tool>("base:item/gold_shovel",     "Gold Shovel",      300, .75f, 512, "base:attribute/shovel"));
		add(std::make_shared<Tool>("base:item/gold_axe",        "Gold Axe",         400, .75f,  64, "base:attribute/axe"));
		add(std::make_shared<Tool>("base:item/diamond_shovel",  "Diamond Shovel",   700,  1.f, 512, "base:attribute/shovel"));
		add(std::make_shared<Tool>("base:item/diamond_axe",     "Diamond Axe",      900,  1.f, 512, "base:attribute/axe"));
		add(std::make_shared<Tool>("base:item/wrench",          "Wrench",            72,  0.f,  -1, "base:attribute/wrench"));

		add(std::make_shared<Floor>("base:item/floor", "Floor", "base:tile/floor", 4, 64));

		add(std::make_shared<Hammer>("base:item/iron_hammer",    "Iron Hammer",    150,  3.f, 128));
		add(std::make_shared<Hammer>("base:item/gold_hammer",    "Gold Hammer",    400, .75f, 128));
		add(std::make_shared<Hammer>("base:item/diamond_hammer", "Diamond Hammer", 900,  1.f, 128));

		add(std::make_shared<Pickaxe>("base:item/iron_pickaxe",    "Iron Pickaxe",     150,  3.f,  64));
		add(std::make_shared<Pickaxe>("base:item/gold_pickaxe",    "Gold Pickaxe",     400, .75f,  64));
		add(std::make_shared<Pickaxe>("base:item/diamond_pickaxe", "Diamond Pickaxe",  900,  1.f, 512));

		add(std::make_shared<Landfill>("base:item/sand",          "Sand",          1, 64, "base:tile/sand"));
		add(std::make_shared<Landfill>("base:item/volcanic_sand", "Volcanic Sand", 3, 64, "base:tile/volcanic_sand"));
		add(std::make_shared<Landfill>("base:item/stone",         "Stone",         1, 64, "base:tile/stone", "base:tile/cave_wall"));
		add(std::make_shared<Landfill>("base:item/grimstone",     "Grimstone",     2, 64, "base:tile/grimstone"));

		add(std::make_shared<PumpItem>("base:item/pump", "Pump", 999, 64)); // TODO: cost
		add(std::make_shared<TankItem>("base:item/tank", "Tank", 999, 64)); // TODO: cost

		add(Furniture::createSimple("base:item/pride_flag",       "Pride Flag",        80, Layer::Highest,   "base:tile/pride_flag"));
		add(Furniture::createSimple("base:item/ace_flag",         "Asexual Flag",      80, Layer::Highest,   "base:tile/ace_flag"));
		add(Furniture::createSimple("base:item/nb_flag",          "Nonbinary Flag",    80, Layer::Highest,   "base:tile/nb_flag"));
		add(Furniture::createSimple("base:item/knives",           "Knives",            10, Layer::Highest,   "base:tile/knives"));
		add(Furniture::createSimple("base:item/kitchen_utensils", "Kitchen Utensils",  10, Layer::Highest,   "base:tile/kitchen_utensils"));
		add(Furniture::createSimple("base:item/plant_pot1",       "Plant Pot",         32, Layer::Submerged, "base:tile/plant1"));
		add(Furniture::createSimple("base:item/plant_pot2",       "Plant Pot",         32, Layer::Submerged, "base:tile/plant2"));
		add(Furniture::createSimple("base:item/plant_pot3",       "Plant Pot",         32, Layer::Submerged, "base:tile/plant3"));
		add(Furniture::createSimple("base:item/electric_guitar",  "Electric Guitar",  100, Layer::Objects,   "base:tile/electric_guitar"));
		add(Furniture::createSimple("base:item/red_planter",      "Red Planter",        8, Layer::Highest,   "base:tile/red_planter"));
		add(Furniture::createSimple("base:item/blue_planter",     "Blue Planter",       8, Layer::Highest,   "base:tile/blue_planter"));
		add(Furniture::createSimple("base:item/cabinet",          "Cabinet",           10, Layer::Objects,   "base:tile/cabinet"));

		add(Furniture::createMarchable("base:item/wooden_wall",     "Wooden Wall",      9, Layer::Objects, "base:tile/wooden_wall",     "base:autotile/wooden_walls"));
		add(Furniture::createMarchable("base:item/tower",           "Tower",           10, Layer::Objects, "base:tile/tower",           "base:autotile/towers"));
		add(Furniture::createMarchable("base:item/kitchen_counter", "Kitchen Counter", 10, Layer::Objects, "base:tile/kitchen_counter", "base:autotile/kitchen_counters"));
		add(Furniture::createMarchable("base:item/dining_table",    "Dining Table",    10, Layer::Objects, "base:tile/dining_table",    "base:autotile/dining_tables"));

		add(Furniture::createTileEntity("base:item/cauldron", "Cauldron", 175, [](const Place &place) -> bool {
			return nullptr != TileEntity::spawn<CraftingStation>(place, "base:tile/red_cauldron_full", place.position, "base:station/cauldron", "base:item/cauldron");
		}));

		add(Furniture::createTileEntity("base:item/purifier", "Purifier", 300, [](const Place &place) -> bool {
			return nullptr != TileEntity::spawn<CraftingStation>(place, "base:tile/purifier", place.position, "base:station/purifier", "base:item/purifier");
		}));

		add(Furniture::createTileEntity("base:item/millstone", "Millstone", 20, [](const Place &place) -> bool {
			return nullptr != TileEntity::spawn<CraftingStation>(place, "base:tile/millstone", place.position, "base:station/millstone", "base:item/millstone");
		}));

		add(Furniture::createTileEntity("base:item/cutting_board", "Cutting Board", 30, [](const Place &place) -> bool {
			return nullptr != TileEntity::spawn<CraftingStation>(place, "base:tile/cutting_board", place.position, "base:station/cutting_board", "base:item/cutting_board");
		}));

		add(Furniture::createTileEntity("base:item/sink", "Sink", 30, [](const Place &place) -> bool {
			return nullptr != TileEntity::spawn<CraftingStation>(place, "base:tile/sink", place.position, "base:station/sink", "base:item/sink");
		}));

		add(Furniture::createTileEntity("base:item/oven", "Oven", 200, [](const Place &place) -> bool {
			return nullptr != TileEntity::spawn<CraftingStation>(place, "base:tile/oven", place.position, "base:station/oven", "base:item/oven");
		}));

		add(Furniture::createTileEntity("base:item/chest", "Chest", 100, [](const Place &place) -> bool {
			auto out = TileEntity::spawn<Chest>(place, "base:tile/chest", place.position, "Chest", "base:item/chest");
			if (!out)
				return false;
			out->setInventory(30);
			return true;
		}));

		add(Furniture::createTileEntity("base:item/fridge", "Fridge", 150, [](const Place &place) -> bool {
			auto out = TileEntity::spawn<Chest>(place, "base:tile/fridge", place.position, "Fridge", "base:item/fridge");
			if (!out)
				return false;
			out->setInventory(30);
			return true;
		}));

		for (int i = 1; i <= 5; ++i) {
			for (const char *color: {"red", "orange", "yellow", "green", "blue", "purple", "white", "black"}) {
				std::string name = std::string(color) + " Flower";
				name[0] = std::toupper(name[0]);
				std::string post = "flower" + std::to_string(i) + "_" + color;
				add(std::make_shared<Plantable>(Identifier("base:item/" + post), std::move(name), Identifier("base:tile/" + post), "base:category/plant_soil", 10)
					->addAttribute("base:attribute/flower")
					->addAttribute(Identifier("base:attribute/flower_" + std::string(color))));
			}
		}

		add(std::make_shared<VoidFlask>("base:item/void_flask", "Void Flask", 128, 1));

		add(std::make_shared<EmptyFlask>("base:item/flask", "Flask", 2, 64));

		add(std::make_shared<FilledFlask>("base:item/water_flask", "Water Flask", 3, "base:fluid/water"));
		add(std::make_shared<FilledFlask>("base:item/lava_flask",  "Lava Flask",  4, "base:fluid/lava"));
		add(std::make_shared<FilledFlask>("base:item/milk_flask",  "Milk Flask",  4, "base:fluid/milk"));
		add(std::make_shared<FilledFlask>("base:item/brine_flask", "Brine Flask", 4, "base:fluid/brine"));

		add(std::make_shared<VoidPickaxe>("base:item/void_pickaxe",    "Void Pickaxe",    1000,  0.f,  -1, "base:attribute/void_pickaxe"));

		add(std::make_shared<CaveEntrance>("base:item/cave_entrance", "Cave Entrance", 50, 1));

		add(std::make_shared<ChemicalItem>("base:item/chemical", "Chemical", 0));

		add(std::make_shared<ItemPipeItem>(4));

		add(std::make_shared<FluidPipeItem>(4));

		add(std::make_shared<CentrifugeItem>("base:item/centrifuge", "Centrifuge", 999, 64)); // TODO: cost

		add(std::make_shared<ContainmentOrb>("base:item/contorb", "Containment Orb", 64, 1)); // TODO: cost

		add(std::make_shared<EnergyPipeItem>(4));

		add(std::make_shared<ChemicalReactorItem>("base:item/chemical_reactor", "Chemical Reactor", 999, 64)); // TODO: cost

		add(std::make_shared<GeothermalGeneratorItem>("base:item/geothermal_generator", "Geothermal Generator", 999, 64)); // TODO: cost

		add(std::make_shared<SnowySapling>("base:item/snowy_sapling", "Snowy Sapling", 5, 64));
		add(std::make_shared<DesertSapling>("base:item/desert_sapling", "Cactus Sapling", 5, 64));
		add(std::make_shared<GrasslandSapling>("base:item/sapling", "Sapling", 5, 64));
	}

	void Game::addEntityFactories() {
		add(EntityFactory::create<Blacksmith>());
		add(EntityFactory::create<Chicken>());
		add(EntityFactory::create<Dog>());
		add(EntityFactory::create<ItemEntity>());
		add(EntityFactory::create<Merchant>());
		add(EntityFactory::create<Miner>());
		add(EntityFactory::create<Pig>());
		add(EntityFactory::create<Sheep>());
		add(EntityFactory::create<Woodcutter>());
	}

	void Game::addTileEntityFactories() {
		add(TileEntityFactory::create<Building>());
		add(TileEntityFactory::create<Centrifuge>());
		add(TileEntityFactory::create<ChemicalReactor>());
		add(TileEntityFactory::create<Chest>());
		add(TileEntityFactory::create<CraftingStation>());
		add(TileEntityFactory::create<GeothermalGenerator>());
		add(TileEntityFactory::create<ItemSpawner>());
		add(TileEntityFactory::create<OreDeposit>());
		add(TileEntityFactory::create<Pipe>());
		add(TileEntityFactory::create<Pump>());
		add(TileEntityFactory::create<Sign>());
		add(TileEntityFactory::create<Stockpile>());
		add(TileEntityFactory::create<Tank>());
		add(TileEntityFactory::create<Teleporter>());
	}

	void Game::addRealms() {
		auto &types = registry<RealmTypeRegistry>();
		auto &factories = registry<RealmFactoryRegistry>();

		auto addRealm = [&]<typename T>(const Identifier &id) {
			types.add(id);
			factories.add(id, std::make_shared<RealmFactory>(RealmFactory::create<T>(id)));
		};

		// ...
		addRealm.operator()<Overworld>(Overworld::ID());
		addRealm.operator()<ShadowRealm>(ShadowRealm::ID());
		addRealm.operator()<House>(House::ID());
		addRealm.operator()<Realm>("base:realm/blacksmith");
		addRealm.operator()<Cave>(Cave::ID());
		addRealm.operator()<Realm>("base:realm/tavern");
		addRealm.operator()<Keep>(Keep::ID());
	}

	void Game::addPacketFactories() {
		add(PacketFactory::create<ProtocolVersionPacket>());
		add(PacketFactory::create<TileEntityPacket>());
		add(PacketFactory::create<ChunkRequestPacket>());
		add(PacketFactory::create<TileUpdatePacket>());
		add(PacketFactory::create<CommandResultPacket>());
		add(PacketFactory::create<CommandPacket>());
		add(PacketFactory::create<SelfTeleportedPacket>());
		add(PacketFactory::create<ChunkTilesPacket>());
		add(PacketFactory::create<RealmNoticePacket>());
		add(PacketFactory::create<LoginPacket>());
		add(PacketFactory::create<LoginStatusPacket>());
		add(PacketFactory::create<RegisterPlayerPacket>());
		add(PacketFactory::create<RegistrationStatusPacket>());
		add(PacketFactory::create<EntityPacket>());
		add(PacketFactory::create<MovePlayerPacket>());
		add(PacketFactory::create<ErrorPacket>());
		add(PacketFactory::create<EntityMovedPacket>());
		add(PacketFactory::create<SendChatMessagePacket>());
		add(PacketFactory::create<EntitySetPathPacket>());
		add(PacketFactory::create<TeleportSelfPacket>());
		add(PacketFactory::create<InteractPacket>());
		add(PacketFactory::create<InventorySlotUpdatePacket>());
		add(PacketFactory::create<DestroyEntityPacket>());
		add(PacketFactory::create<InventoryPacket>());
		add(PacketFactory::create<SetActiveSlotPacket>());
		add(PacketFactory::create<ActiveSlotSetPacket>());
		add(PacketFactory::create<DestroyTileEntityPacket>());
		add(PacketFactory::create<ClickPacket>());
		add(PacketFactory::create<TimePacket>());
		add(PacketFactory::create<CraftPacket>());
		add(PacketFactory::create<ContinuousInteractionPacket>());
		add(PacketFactory::create<FluidUpdatePacket>());
		add(PacketFactory::create<HeldItemSetPacket>());
		add(PacketFactory::create<SetHeldItemPacket>());
		add(PacketFactory::create<EntityRequestPacket>());
		add(PacketFactory::create<TileEntityRequestPacket>());
		add(PacketFactory::create<JumpPacket>());
		add(PacketFactory::create<DropItemPacket>());
		add(PacketFactory::create<OpenModuleForAgentPacket>());
		add(PacketFactory::create<SwapSlotsPacket>());
		add(PacketFactory::create<MoveSlotsPacket>());
		add(PacketFactory::create<AgentMessagePacket>());
		add(PacketFactory::create<SetTileEntityEnergyPacket>());
		add(PacketFactory::create<SetPlayerStationTypesPacket>());
		add(PacketFactory::create<EntityChangingRealmsPacket>());
		add(PacketFactory::create<ChatMessageSentPacket>());
		add(PacketFactory::create<OpenTextTabPacket>());
		add(PacketFactory::create<DragPacket>());
	}

	void Game::addLocalCommandFactories() {
		add(LocalCommandFactory::create<RegisterCommand>());
		add(LocalCommandFactory::create<LoginCommand>());
		add(LocalCommandFactory::create<UsageCommand>());
		add(LocalCommandFactory::create<ChemicalCommand>());
		add(LocalCommandFactory::create<PlayersCommand>());
		add(LocalCommandFactory::create<SegfaultCommand>());
	}

	void Game::addTiles() {
		auto &reg = registry<TileRegistry>();

		reg.add<FarmlandTile>();
		reg.add<ForestFloorTile>();
		reg.addMineable("base:tile/stone", ItemStack(*this, "base:item/stone"), true);
		reg.add("base:tile/cave_coal",     std::make_shared<CaveTile>("base:tile/cave_coal",     ItemStack(*this, "base:item/coal"),        "base:tile/cave_dirt"));
		reg.add("base:tile/cave_copper",   std::make_shared<CaveTile>("base:tile/cave_copper",   ItemStack(*this, "base:item/copper_ore"),  "base:tile/cave_dirt"));
		reg.add("base:tile/cave_diamond",  std::make_shared<CaveTile>("base:tile/cave_diamond",  ItemStack(*this, "base:item/diamond_ore"), "base:tile/cave_dirt"));
		reg.add("base:tile/cave_gold",     std::make_shared<CaveTile>("base:tile/cave_gold",     ItemStack(*this, "base:item/gold_ore"),    "base:tile/cave_dirt"));
		reg.add("base:tile/cave_iron",     std::make_shared<CaveTile>("base:tile/cave_iron",     ItemStack(*this, "base:item/iron_ore"),    "base:tile/cave_dirt"));
		reg.add("base:tile/cave_wall",     std::make_shared<CaveTile>("base:tile/cave_wall",     ItemStack(*this, "base:item/stone"),       "base:tile/cave_dirt"));
		reg.add("base:tile/grimstone",     std::make_shared<CaveTile>("base:tile/grimstone",     ItemStack(*this, "base:item/grimstone"),   "base:tile/grimdirt"));
		reg.add("base:tile/grim_diamond",  std::make_shared<CaveTile>("base:tile/grim_diamond",  ItemStack(*this, "base:item/diamond_ore"), "base:tile/grimdirt"));
		reg.add("base:tile/grim_uranium",  std::make_shared<CaveTile>("base:tile/grim_uranium",  ItemStack(*this, "base:item/uranium_ore"), "base:tile/grimdirt"));
		reg.add("base:tile/grim_fireopal", std::make_shared<CaveTile>("base:tile/grim_fireopal", ItemStack(*this, "base:item/fire_opal"),   "base:tile/grimdirt"));

		const auto monomap = registry<TilesetRegistry>().at("base:tileset/monomap");
		auto grass = std::make_shared<GrassTile>();
		for (const auto &tilename: monomap->getTilesByCategory("base:category/flower_spawners"))
			reg.add(tilename, grass);

		for (const auto &[crop_name, crop]: registry<CropRegistry>()) {
			if (crop->customType.empty()) {
				auto tile = std::make_shared<CropTile>(crop);
				for (const auto &stage: crop->stages)
					reg.add(stage, tile);
			} else if (crop->customType == "base:tile/tree") {
				// TODO: We need the factory thing.
				auto tile = std::make_shared<TreeTile>(crop);
				for (const auto &stage: crop->stages)
					reg.add(stage, tile);
			}
		}
	}

	void Game::addModuleFactories() {
		add(ModuleFactory::create<ExternalInventoryModule>());
		add(ModuleFactory::create<FluidLevelsModule>());
		add(ModuleFactory::create<ChemicalReactorModule>());
		add(ModuleFactory::create<EnergyLevelModule>());
	}

	void Game::initialSetup(const std::filesystem::path &dir) {
		initRegistries();
		addItems();
		traverseData(dataRoot / dir);
		addRealms();
		addEntityFactories();
		addTileEntityFactories();
		addPacketFactories();
		addLocalCommandFactories();
		addTiles();
		addModuleFactories();
	}

	void Game::initEntities() {
		for (const auto &[realm_id, realm]: realms)
			realm->initEntities();
	}

	void Game::initInteractionSets() {
		interactionSets.clear();
		auto standard = std::make_shared<StandardInteractions>();
		for (const auto &type: registry<RealmTypeRegistry>().items)
			interactionSets.emplace(type, standard);
	}

	void Game::add(std::shared_ptr<Item> item) {
		registry<ItemRegistry>().add(item->identifier, item);
		for (const auto &attribute: item->attributes)
			itemsByAttribute[attribute].insert(item);
	}

	void Game::add(EntityFactory &&factory) {
		auto shared = std::make_shared<EntityFactory>(std::move(factory));
		registry<EntityFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::add(TileEntityFactory &&factory) {
		auto shared = std::make_shared<TileEntityFactory>(std::move(factory));
		registry<TileEntityFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::add(RealmFactory &&factory) {
		auto shared = std::make_shared<RealmFactory>(std::move(factory));
		registry<RealmFactoryRegistry>().add(shared->identifier, shared);
	}

	void Game::add(PacketFactory &&factory) {
		auto shared = std::make_shared<PacketFactory>(std::move(factory));
		registry<PacketFactoryRegistry>().add(shared->number, shared);
	}

	void Game::add(LocalCommandFactory &&factory) {
		auto shared = std::make_shared<LocalCommandFactory>(std::move(factory));
		registry<LocalCommandFactoryRegistry>().add(shared->name, shared);
	}

	void Game::add(ModuleFactory &&factory) {
		auto shared = std::make_shared<ModuleFactory>(std::move(factory));
		registry<ModuleFactoryRegistry>().add(shared->identifier, shared);
	}

	struct DependencyNode {
		std::string name;
		bool isCategory;
	};

	void Game::traverseData(const std::filesystem::path &dir) {
		std::vector<std::filesystem::path> json_paths;
		// A -> B means A is loaded before B.
		Graph<DependencyNode> dependencies;
		// Maps data types like "base:crop_map" to vectors of names of data files that contain instances of them.
		std::unordered_map<std::string, std::vector<std::string>> categories;
		std::unordered_map<std::string, nlohmann::json> jsons;

		auto add_dependencies = [&](const nlohmann::json &json) {
			const std::string name = json.at("name");

			bool created{};
			auto &node = dependencies.get(name, created);
			if (created)
				node.data = {name, false};

			for (const auto &dependency_json: json.at("dependencies")) {
				const std::string order = dependency_json.at(0);
				const bool is_after = order == "after";
				if (!is_after && order != "before")
					throw std::runtime_error("Couldn't load JSON: invalid order \"" + order + '"');

				const std::string specifier = dependency_json.at(1);
				const bool is_type = specifier == "type";
				if (!is_type && specifier != "name")
					throw std::runtime_error("Couldn't load JSON: invalid specifier \"" + specifier + '"');

				const std::string id = dependency_json.at(2);
				const bool is_category = id.find(':') != std::string::npos;

				auto &other_node = dependencies.get(id, created);
				if (created)
					other_node.data = {id, is_category};

				if (is_after)
					other_node.link(node);
				else
					node.link(other_node);
			}
		};

		std::function<void(const std::filesystem::path &)> traverse = [&](const auto &dir) {
			for (const auto &entry: std::filesystem::directory_iterator(dir)) {
				if (entry.is_directory()) {
					traverse(entry.path());
				} else if (entry.is_regular_file()) {
					if (auto path = entry.path(); path.extension() == ".json")
						json_paths.push_back(std::move(path));
				}
			}
		};

		traverse(dir);

		for (const auto &path: json_paths) {
			std::ifstream ifs(path);
			std::stringstream ss;
			ss << ifs.rdbuf();
			nlohmann::json json = nlohmann::json::parse(ss.str());
			add_dependencies(json);
			std::string name = json.at("name");
			for (const nlohmann::json &item: json.at("data"))
				categories[item.at(0)].push_back(name);
			jsons.emplace(std::move(name), std::move(json));
		}

		for (const auto &[category, names]: categories) {
			auto *category_node = dependencies.maybe(category);
			if (category_node == nullptr)
				continue; // ???

			for (const std::string &name: names) {
				auto &name_node = dependencies[name];

				for (const auto &in: category_node->getIn())
					in->link(name_node);

				for (const auto &out: category_node->getOut())
					name_node.link(*out);
			}

		}

		for (const auto &[category, names]: categories)
			if (dependencies.hasLabel(category))
				dependencies -= category;

		for (const auto &node: dependencies.topoSort()) {
			assert(!node->data.isCategory);
			for (const nlohmann::json &json: jsons.at(node->data.name).at("data"))
				loadData(json);
		}
	}

	void Game::loadData(const nlohmann::json &json) {
		Identifier type = json.at(0);

		// TODO: make a map of handlers for different types instead of if-elsing here
		if (type == "base:entity_texture_map") {

			auto &textures = registry<EntityTextureRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				textures.add(Identifier(key), EntityTexture(Identifier(key), value.at(0), value.at(1)));

		} else if (type == "base:ore_map") {

			auto &ores = registry<OreRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				ores.add(Identifier(key), Ore(Identifier(key), ItemStack::fromJSON(*this, value.at(0)), value.at(1), value.at(2), value.at(3), value.at(4), value.at(5)));

		} else if (type == "base:realm_details_map") {

			auto &details = registry<RealmDetailsRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				details.add(Identifier(key), RealmDetails(Identifier(key), value.at("tileset")));

		} else if (type == "base:texture_map") {

			auto &textures = registry<TextureRegistry>();
			for (const auto &[key, value]: json.at(1).items()) {
				if (value.size() == 1)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0)))->init();
				else if (value.size() == 2)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0), value.at(1)))->init();
				else if (value.size() == 3)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0), value.at(1), value.at(2)))->init();
				else
					throw std::invalid_argument("Expected Texture JSON size to be 1, 2 or 3, not " + std::to_string(value.size()));
			}

		} else if (type == "base:tileset") {

			Identifier identifier = json.at(1);
			std::filesystem::path base_dir = json.at(2);
			auto &tilesets = registry<TilesetRegistry>();
			tilesets.add(identifier, tileStitcher(base_dir, identifier));

		} else if (type == "base:itemset") {

			Identifier identifier = json.at(1);
			std::filesystem::path base_dir = json.at(2);
			auto &itemsets = registry<ItemSetRegistry>();
			itemsets.add(identifier, itemStitcher(&registry<ItemTextureRegistry>(), base_dir, identifier));

		} else if (type == "base:recipe_list") {

			for (const auto &recipe_json: json.at(1))
				addRecipe(recipe_json);

		} else if (type == "base:fluid_list") {

			auto &fluids = registry<FluidRegistry>();
			for (const auto &pair: json.at(1)) {
				const Identifier fluid_name = pair.at(0);
				const nlohmann::json value = pair.at(1);
				if (auto iter = value.find("flask"); iter != value.end())
					fluids.add(fluid_name, Fluid(fluid_name, value.at("name"), value.at("tileset"), value.at("tilename"), *iter));
				else
					fluids.add(fluid_name, Fluid(fluid_name, value.at("name"), value.at("tileset"), value.at("tilename")));
			}

		} else if (type == "base:crop_map") {

			auto &crops = registry<CropRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				crops.add(Identifier(key), Crop(Identifier(key), *this, value));

		} else if (type.getPathStart() == "ignore") {

			// For old data that isn't ready to be removed yet.

		} else
			throw std::runtime_error("Unknown data file type: " + type.str());
	}

	void Game::addRecipe(const nlohmann::json &json) {
		const Identifier identifier = json.at(0);
		if (identifier.getPathStart() != "ignore")
			registries.at(identifier)->toUnnamed()->add(*this, json.at(1));
	}

	RealmID Game::newRealmID() const {
		// TODO: a less stupid way of doing this.
		RealmID max = 1;
		for (const auto &[id, realm]: realms)
			max = std::max(max, id);
		return max + 1;
	}

	double Game::getTotalSeconds() const {
		return time;
	}

	double Game::getHour() const {
		const auto base = time / 10. + hourOffset;
		return int64_t(base) % 24 + fractional(base);
	}

	double Game::getMinute() const {
		return 60. * fractional(getHour());
	}

	double Game::getDivisor() const {
		return 3. - 2. * sin(getHour() * 3.1415926 / 24.);
	}

	std::optional<TileID> Game::getFluidTileID(FluidID fluid_id) {
		if (auto iter = fluidCache.find(fluid_id); iter != fluidCache.end())
			return iter->second;

		if (auto fluid = registry<FluidRegistry>().maybe(static_cast<size_t>(fluid_id))) {
			if (auto tileset = registry<TilesetRegistry>().maybe(fluid->tilesetName)) {
				if (auto fluid_tileid = tileset->maybe(fluid->tilename)) {
					fluidCache.emplace(fluid_id, *fluid_tileid);
					return *fluid_tileid;
				}
			}
		}

		return std::nullopt;
	}

	std::shared_ptr<Fluid> Game::getFluid(FluidID fluid_id) const {
		return registry<FluidRegistry>().maybe(static_cast<size_t>(fluid_id));
	}

	std::shared_ptr<Tile> Game::getTile(const Identifier &identifier) {
		auto &reg = registry<TileRegistry>();
		if (auto found = reg.maybe(identifier))
			return found;
		static auto default_tile = std::make_shared<Tile>("base:tile/?");
		return default_tile;
	}

	RealmPtr Game::tryRealm(RealmID realm_id) const {
		auto lock = realms.sharedLock();
		if (auto iter = realms.find(realm_id); iter != realms.end())
			return iter->second;
		return {};
	}

	RealmPtr Game::getRealm(RealmID realm_id) const {
		auto lock = realms.sharedLock();
		return realms.at(realm_id);
	}

	RealmPtr Game::getRealm(RealmID realm_id, const std::function<RealmPtr()> &creator) {
		auto lock = realms.uniqueLock();
		if (auto iter = realms.find(realm_id); iter != realms.end())
			return iter->second;
		RealmPtr new_realm = creator();
		realms.emplace(realm_id, new_realm);
		return new_realm;
	}

	void Game::addRealm(RealmID realm_id, RealmPtr realm) {
		auto lock = realms.uniqueLock();
		if (!realms.emplace(realm_id, realm).second)
			throw std::runtime_error("Couldn't add realm " + std::to_string(realm_id) + ": a realm with that ID already exists");
	}

	void Game::addRealm(RealmPtr realm) {
		addRealm(realm->id, realm);
	}

	bool Game::hasRealm(RealmID realm_id) const {
		auto lock = realms.sharedLock();
		return realms.contains(realm_id);
	}

	void Game::removeRealm(RealmID realm_id) {
		RealmPtr realm = getRealm(realm_id);
		realm->onRemove();
		auto lock = realms.uniqueLock();
		realms.erase(realm_id);
	}

	GamePtr Game::create(Side side, const GameArgument &argument) {
		GamePtr out;
		if (side == Side::Client)
			out = GamePtr(new ClientGame(*std::get<Canvas *>(argument)));
		else
			out = GamePtr(new ServerGame(std::get<std::shared_ptr<Server>>(argument)));
		out->initialSetup();
		return out;
	}

	GamePtr Game::fromJSON(Side side, const nlohmann::json &json, const GameArgument &argument) {
		auto out = create(side, argument);
		out->initialSetup();
		{
			auto lock = out->realms.uniqueLock();
			for (const auto &[string, realm_json]: json.at("realms").get<std::unordered_map<std::string, nlohmann::json>>())
				out->realms.emplace(parseUlong(string), Realm::fromJSON(*out, realm_json));
		}
		out->hourOffset = json.contains("hourOffset")? json.at("hourOffset").get<float>() : 0.f;
		out->debugMode = json.contains("debugMode")? json.at("debugMode").get<bool>() : false;
		out->cavesGenerated = json.contains("cavesGenerated")? json.at("cavesGenerated").get<decltype(Game::cavesGenerated)>() : 0;
		return out;
	}

	ClientGame & Game::toClient() {
		return dynamic_cast<ClientGame &>(*this);
	}

	const ClientGame & Game::toClient() const {
		return dynamic_cast<const ClientGame &>(*this);
	}

	std::shared_ptr<ClientGame> Game::toClientPointer() {
		assert(getSide() == Side::Client);
		return std::static_pointer_cast<ClientGame>(shared_from_this());
	}

	ServerGame & Game::toServer() {
		return dynamic_cast<ServerGame &>(*this);
	}

	const ServerGame & Game::toServer() const {
		return dynamic_cast<const ServerGame &>(*this);
	}

	std::shared_ptr<ServerGame> Game::toServerPointer() {
		assert(getSide() == Side::Server);
		return std::static_pointer_cast<ServerGame>(shared_from_this());
	}

	void to_json(nlohmann::json &json, const Game &game) {
		json["debugMode"] = game.debugMode;
		json["realms"] = std::unordered_map<std::string, nlohmann::json>();
		game.iterateRealms([&](const RealmPtr &realm) {
			realm->toJSON(json["realms"][std::to_string(realm->id)], true);
		});
		json["hourOffset"] = game.getHour();
		if (0 < game.cavesGenerated)
			json["cavesGenerated"] = game.cavesGenerated;
	}

	template <>
	std::shared_ptr<Agent> Game::getAgent<Agent>(GlobalID gid) {
		auto shared_lock = allAgents.sharedLock();
		if (auto iter = allAgents.find(gid); iter != allAgents.end()) {
			if (auto agent = iter->second.lock())
				return agent;
			// This should *probably* not result in a data race in practice...
			shared_lock.unlock();
			auto unique_lock = allAgents.uniqueLock();
			allAgents.erase(gid);
		}

		return nullptr;
	}
}
