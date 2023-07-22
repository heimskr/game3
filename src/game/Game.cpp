#include <fstream>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "Tileset.h"
#include "command/local/LocalCommandFactory.h"
#include "command/local/LoginCommand.h"
#include "command/local/RegisterCommand.h"
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
#include "item/Bomb.h"
#include "item/CaveEntrance.h"
#include "item/Floor.h"
#include "item/Furniture.h"
#include "item/Hammer.h"
#include "item/Hoe.h"
#include "item/Item.h"
#include "item/ItemPipeItem.h"
#include "item/Landfill.h"
#include "item/Landfills.h"
#include "item/Mushroom.h"
#include "item/Pickaxe.h"
#include "item/Plantable.h"
#include "item/Sapling.h"
#include "item/Seed.h"
#include "item/Tool.h"
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
#include "packet/StartPlayerMovementPacket.h"
#include "packet/ErrorPacket.h"
#include "packet/EntityMovedPacket.h"
#include "packet/StopPlayerMovementPacket.h"
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
#include "packet/OpenAgentInventoryPacket.h"
#include "packet/SwapSlotsPacket.h"
#include "realm/Cave.h"
#include "realm/House.h"
#include "realm/Keep.h"
#include "realm/Overworld.h"
#include "realm/RealmFactory.h"
#include "recipe/CraftingRecipe.h"
#include "registry/Registries.h"
#include "tile/CropTile.h"
#include "tile/ForestFloorTile.h"
#include "tile/GrassTile.h"
#include "tile/Tile.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Ghost.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Pipe.h"
#include "tileentity/Sign.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Teleporter.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "tileentity/Tree.h"
#include "util/AStar.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	void Game::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1'000'000'000.;
		time += delta;
		++currentTick;
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
		registries.add<GhostDetailsRegistry>();
		registries.add<GhostFunctionRegistry>();
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
		// TODO: plugins
	}

	void Game::addItems() {
		add(std::make_shared<Hoe>("base:item/iron_hoe", "Iron Hoe", 85, 128));

		add(std::make_shared<Bomb>("base:item/bomb", "Bomb", 32, 64));

		add(std::make_shared<Item>("base:item/shortsword",     "Shortsword",     100,  1));
		add(std::make_shared<Item>("base:item/red_potion",     "Red Potion",      20,  8));
		add(std::make_shared<Item>("base:item/coins",          "Gold",             1, 1'000'000));
		add(std::make_shared<Item>("base:item/iron_ore",       "Iron Ore",        10, 64));
		add(std::make_shared<Item>("base:item/copper_ore",     "Copper Ore",       8, 64));
		add(std::make_shared<Item>("base:item/gold_ore",       "Gold Ore",        20, 64));
		add(std::make_shared<Item>("base:item/diamond_ore",    "Diamond Ore",     80, 64));
		add(std::make_shared<Item>("base:item/diamond",        "Diamond",        100, 64));
		add(std::make_shared<Item>("base:item/coal",           "Coal",             5, 64));
		add(std::make_shared<Item>("base:item/oil",            "Oil",             15, 64));
		add(std::make_shared<Item>("base:item/wood",           "Wood",             3, 64));
		add(std::make_shared<Item>("base:item/stone",          "Stone",            1, 64));
		add(std::make_shared<Item>("base:item/iron_bar",       "Iron Bar",        16, 64));
		add(std::make_shared<Item>("base:item/gold_bar",       "Gold Bar",        45, 64));
		add(std::make_shared<Item>("base:item/plank",          "Plank",            4, 64));
		add(std::make_shared<Item>("base:item/dirt",           "Dirt",             1, 64));
		add(std::make_shared<Item>("base:item/brick",          "Brick",            3, 64));
		add(std::make_shared<Item>("base:item/pot",            "Pot",             24, 64));
		add(std::make_shared<Item>("base:item/mead",           "Mead",            10, 16));
		add(std::make_shared<Item>("base:item/honey",          "Honey",            5, 64));
		add(std::make_shared<Item>("base:item/ash",            "Ash",              1, 64));
		add(std::make_shared<Item>("base:item/silicon",        "Silicon",          2, 64));
		add(std::make_shared<Item>("base:item/electronics",    "Electronics",     32, 64));
		add(std::make_shared<Item>("base:item/sulfur",         "Sulfur",          15, 64));
		add(std::make_shared<Item>("base:item/cotton",         "Cotton",           8, 64));
		add(std::make_shared<Item>("base:item/red_dye",        "Red Dye",         12, 64));
		add(std::make_shared<Item>("base:item/orange_dye",     "Orange Dye",      12, 64));
		add(std::make_shared<Item>("base:item/yellow_dye",     "Yellow Dye",      12, 64));
		add(std::make_shared<Item>("base:item/green_dye",      "Green Dye",       12, 64));
		add(std::make_shared<Item>("base:item/blue_dye",       "Blue Dye",        12, 64));
		add(std::make_shared<Item>("base:item/purple_dye",     "Purple Dye",      12, 64));
		add(std::make_shared<Item>("base:item/white_dye",      "White Dye",       12, 64));
		add(std::make_shared<Item>("base:item/black_dye",      "Black Dye",       12, 64));
		add(std::make_shared<Item>("base:item/brown_dye",      "Brown Dye",       12, 64));
		add(std::make_shared<Item>("base:item/pink_dye",       "Pink Dye",        12, 64));
		add(std::make_shared<Item>("base:item/light_blue_dye", "Light Blue Dye",  12, 64));
		add(std::make_shared<Item>("base:item/gray_dye",       "Gray Dye",        12, 64));
		add(std::make_shared<Item>("base:item/lime_dye",       "Lime Dye",        12, 64));

		add(std::make_shared<Seed>("base:item/cotton_seeds",   "Cotton Seeds",   "base:tile/cotton_0"_id, 4));

		add(std::make_shared<Tool>("base:item/iron_axe",       "Iron Axe",       150,  3.f, 128, "base:attribute/axe"_id));
		add(std::make_shared<Tool>("base:item/iron_shovel",    "Iron Shovel",    120,  3.f,  64, "base:attribute/shovel"_id));
		add(std::make_shared<Tool>("base:item/gold_axe",       "Gold Axe",       400, .75f,  64, "base:attribute/axe"_id));
		add(std::make_shared<Tool>("base:item/gold_shovel",    "Gold Shovel",    300, .75f, 512, "base:attribute/shovel"_id));
		add(std::make_shared<Tool>("base:item/diamond_axe",    "Diamond Axe",    900,  1.f, 512, "base:attribute/axe"_id));
		add(std::make_shared<Tool>("base:item/diamond_shovel", "Diamond Shovel", 700,  1.f, 512, "base:attribute/shovel"_id));
		add(std::make_shared<Tool>("base:item/wrench",         "Wrench",          72,  0.f,  -1, "base:attribute/wrench"_id));

		add(std::make_shared<Floor>("base:item/floor", "Floor", "base:tile/floor", 4, 64));

		add(std::make_shared<Hammer>("base:item/iron_hammer",    "Iron Hammer",    150,  3.f, 128));
		add(std::make_shared<Hammer>("base:item/gold_hammer",    "Gold Hammer",    400, .75f, 128));
		add(std::make_shared<Hammer>("base:item/diamond_hammer", "Diamond Hammer", 900,  1.f, 128));

		add(std::make_shared<Pickaxe>("base:item/iron_pickaxe",    "Iron Pickaxe",    150,  3.f,  64, "base:attribute/pickaxe"_id));
		add(std::make_shared<Pickaxe>("base:item/gold_pickaxe",    "Gold Pickaxe",    400, .75f,  64, "base:attribute/pickaxe"_id));
		add(std::make_shared<Pickaxe>("base:item/diamond_pickaxe", "Diamond Pickaxe", 900,  1.f, 512, "base:attribute/pickaxe"_id));

		add(std::make_shared<Sapling>("base:item/sapling", "Sapling", 5, 64));

		add(std::make_shared<Landfill>("base:item/sand",          "Sand",          1, 64, "base:tileset/monomap", "base:tile/shallow_water", Landfill::DEFAULT_COUNT, "base:tile/sand"));
		add(std::make_shared<Landfill>("base:item/volcanic_sand", "Volcanic Sand", 3, 64, "base:tileset/monomap", "base:tile/shallow_water", Landfill::DEFAULT_COUNT, "base:tile/volcanic_sand"));
		add(std::make_shared<Landfill>("base:item/clay",          "Clay",          2, 64, clayRequirement));

		add(std::make_shared<Mushroom>("base:item/saffron_milkcap", "Saffron Milkcap",    10, 1 ));
		add(std::make_shared<Mushroom>("base:item/honey_fungus",    "Honey Fungus",       15, 18));
		add(std::make_shared<Mushroom>("base:item/brittlegill",     "Golden Brittlegill", 20, 7 ));
		add(std::make_shared<Mushroom>("base:item/indigo_milkcap",  "Indigo Milkcap",     20, 11));
		add(std::make_shared<Mushroom>("base:item/black_trumpet",   "Black Trumpet",      20, 29));
		add(std::make_shared<Mushroom>("base:item/grey_knight",     "Grey Knight",        20, 12));

		add(std::make_shared<Furniture>("base:item/wooden_wall", "Wooden Wall",   9, 64));
		add(std::make_shared<Furniture>("base:item/plant_pot1",  "Plant Pot",    32, 64));
		add(std::make_shared<Furniture>("base:item/plant_pot2",  "Plant Pot",    32, 64));
		add(std::make_shared<Furniture>("base:item/plant_pot3",  "Plant Pot",    32, 64));
		add(std::make_shared<Furniture>("base:item/tower",       "Tower",        10, 64));
		add(std::make_shared<Furniture>("base:item/pride_flag",  "Pride Flag",   80, 64));
		add(std::make_shared<Furniture>("base:item/cauldron",    "Cauldron",    175,  1));
		add(std::make_shared<Furniture>("base:item/purifier",    "Purifier",    300,  1));

		add(std::make_shared<Plantable>("base:item/flower1_red",    "Red Flower",    "base:tile/flower1_red",    "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_red"));
		add(std::make_shared<Plantable>("base:item/flower1_orange", "Orange Flower", "base:tile/flower1_orange", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_orange"));
		add(std::make_shared<Plantable>("base:item/flower1_yellow", "Yellow Flower", "base:tile/flower1_yellow", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_yellow"));
		add(std::make_shared<Plantable>("base:item/flower1_green",  "Green Flower",  "base:tile/flower1_green",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_green"));
		add(std::make_shared<Plantable>("base:item/flower1_blue",   "Blue Flower",   "base:tile/flower1_blue",   "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_blue"));
		add(std::make_shared<Plantable>("base:item/flower1_purple", "Purple Flower", "base:tile/flower1_purple", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_purple"));
		add(std::make_shared<Plantable>("base:item/flower1_white",  "White Flower",  "base:tile/flower1_white",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_white"));
		add(std::make_shared<Plantable>("base:item/flower1_black",  "Black Flower",  "base:tile/flower1_black",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_black"));
		add(std::make_shared<Plantable>("base:item/flower2_red",    "Red Flower",    "base:tile/flower2_red",    "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_red"));
		add(std::make_shared<Plantable>("base:item/flower2_orange", "Orange Flower", "base:tile/flower2_orange", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_orange"));
		add(std::make_shared<Plantable>("base:item/flower2_yellow", "Yellow Flower", "base:tile/flower2_yellow", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_yellow"));
		add(std::make_shared<Plantable>("base:item/flower2_green",  "Green Flower",  "base:tile/flower2_green",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_green"));
		add(std::make_shared<Plantable>("base:item/flower2_blue",   "Blue Flower",   "base:tile/flower2_blue",   "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_blue"));
		add(std::make_shared<Plantable>("base:item/flower2_purple", "Purple Flower", "base:tile/flower2_purple", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_purple"));
		add(std::make_shared<Plantable>("base:item/flower2_white",  "White Flower",  "base:tile/flower2_white",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_white"));
		add(std::make_shared<Plantable>("base:item/flower2_black",  "Black Flower",  "base:tile/flower2_black",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_black"));
		add(std::make_shared<Plantable>("base:item/flower3_red",    "Red Flower",    "base:tile/flower3_red",    "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_red"));
		add(std::make_shared<Plantable>("base:item/flower3_orange", "Orange Flower", "base:tile/flower3_orange", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_orange"));
		add(std::make_shared<Plantable>("base:item/flower3_yellow", "Yellow Flower", "base:tile/flower3_yellow", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_yellow"));
		add(std::make_shared<Plantable>("base:item/flower3_green",  "Green Flower",  "base:tile/flower3_green",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_green"));
		add(std::make_shared<Plantable>("base:item/flower3_blue",   "Blue Flower",   "base:tile/flower3_blue",   "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_blue"));
		add(std::make_shared<Plantable>("base:item/flower3_purple", "Purple Flower", "base:tile/flower3_purple", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_purple"));
		add(std::make_shared<Plantable>("base:item/flower3_white",  "White Flower",  "base:tile/flower3_white",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_white"));
		add(std::make_shared<Plantable>("base:item/flower3_black",  "Black Flower",  "base:tile/flower3_black",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_black"));
		add(std::make_shared<Plantable>("base:item/flower4_red",    "Red Flower",    "base:tile/flower4_red",    "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_red"));
		add(std::make_shared<Plantable>("base:item/flower4_orange", "Orange Flower", "base:tile/flower4_orange", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_orange"));
		add(std::make_shared<Plantable>("base:item/flower4_yellow", "Yellow Flower", "base:tile/flower4_yellow", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_yellow"));
		add(std::make_shared<Plantable>("base:item/flower4_green",  "Green Flower",  "base:tile/flower4_green",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_green"));
		add(std::make_shared<Plantable>("base:item/flower4_blue",   "Blue Flower",   "base:tile/flower4_blue",   "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_blue"));
		add(std::make_shared<Plantable>("base:item/flower4_purple", "Purple Flower", "base:tile/flower4_purple", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_purple"));
		add(std::make_shared<Plantable>("base:item/flower4_white",  "White Flower",  "base:tile/flower4_white",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_white"));
		add(std::make_shared<Plantable>("base:item/flower4_black",  "Black Flower",  "base:tile/flower4_black",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_black"));
		add(std::make_shared<Plantable>("base:item/flower5_red",    "Red Flower",    "base:tile/flower5_red",    "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_red"));
		add(std::make_shared<Plantable>("base:item/flower5_orange", "Orange Flower", "base:tile/flower5_orange", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_orange"));
		add(std::make_shared<Plantable>("base:item/flower5_yellow", "Yellow Flower", "base:tile/flower5_yellow", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_yellow"));
		add(std::make_shared<Plantable>("base:item/flower5_green",  "Green Flower",  "base:tile/flower5_green",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_green"));
		add(std::make_shared<Plantable>("base:item/flower5_blue",   "Blue Flower",   "base:tile/flower5_blue",   "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_blue"));
		add(std::make_shared<Plantable>("base:item/flower5_purple", "Purple Flower", "base:tile/flower5_purple", "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_purple"));
		add(std::make_shared<Plantable>("base:item/flower5_white",  "White Flower",  "base:tile/flower5_white",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_white"));
		add(std::make_shared<Plantable>("base:item/flower5_black",  "Black Flower",  "base:tile/flower5_black",  "base:category/plant_soil", 10)->addAttribute("base:attribute/flower")->addAttribute("base:attribute/flower_black"));

		add(std::make_shared<CaveEntrance>("base:item/cave_entrance", "Cave Entrance", 50, 1));

		add(std::make_shared<ItemPipeItem>(4));
	}

	void Game::addGhosts() {
		Game3::initGhosts(*this);
	}

	void Game::addEntityFactories() {
		add(EntityFactory::create<Blacksmith>());
		add(EntityFactory::create<ItemEntity>());
		add(EntityFactory::create<Merchant>());
		add(EntityFactory::create<Miner>());
		add(EntityFactory::create<Woodcutter>());
		add(EntityFactory::create<Sheep>());
		add(EntityFactory::create<Pig>());
		add(EntityFactory::create<Chicken>());
		add(EntityFactory::create<Dog>());
	}

	void Game::addTileEntityFactories() {
		add(TileEntityFactory::create<Building>());
		add(TileEntityFactory::create<Chest>());
		add(TileEntityFactory::create<CraftingStation>());
		add(TileEntityFactory::create<Ghost>());
		add(TileEntityFactory::create<ItemSpawner>());
		add(TileEntityFactory::create<OreDeposit>());
		add(TileEntityFactory::create<Sign>());
		add(TileEntityFactory::create<Stockpile>());
		add(TileEntityFactory::create<Teleporter>());
		add(TileEntityFactory::create<Tree>());
		add(TileEntityFactory::create<Pipe>());
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
		addRealm.operator()<House>(House::ID());
		addRealm.operator()<Realm>("base:realm/blacksmith"_id);
		addRealm.operator()<Cave>(Cave::ID());
		addRealm.operator()<Realm>("base:realm/tavern"_id);
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
		add(PacketFactory::create<StartPlayerMovementPacket>());
		add(PacketFactory::create<ErrorPacket>());
		add(PacketFactory::create<EntityMovedPacket>());
		add(PacketFactory::create<StopPlayerMovementPacket>());
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
		add(PacketFactory::create<OpenAgentInventoryPacket>());
		add(PacketFactory::create<SwapSlotsPacket>());
	}

	void Game::addLocalCommandFactories() {
		add(LocalCommandFactory::create<RegisterCommand>());
		add(LocalCommandFactory::create<LoginCommand>());
		add(LocalCommandFactory::create<UsageCommand>());
	}

	void Game::addTiles() {
		auto &reg = registry<TileRegistry>();
		reg.add<ForestFloorTile>();

		const auto monomap = registry<TilesetRegistry>().at("base:tileset/monomap"_id);
		auto grass = std::make_shared<GrassTile>();
		for (const auto &tilename: monomap->getTilesByCategory("base:category/flower_spawners"_id))
			reg.add(tilename, grass);

		for (const auto &[crop_name, crop]: registry<CropRegistry>()) {
			auto tile = std::make_shared<CropTile>(crop);
			for (const auto &stage: crop->stages)
				reg.add(stage, tile);
		}
	}

	void Game::initialSetup(const std::filesystem::path &dir) {
		initRegistries();
		addItems();
		traverseData(dir);
		addGhosts();
		addRealms();
		addEntityFactories();
		addTileEntityFactories();
		addPacketFactories();
		addLocalCommandFactories();
		addTiles();
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

	void Game::add(std::shared_ptr<GhostDetails> details) {
		registry<GhostDetailsRegistry>().add(details->identifier, details);
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

	void Game::add(GhostFunction &&function) {
		auto shared = std::make_shared<GhostFunction>(std::move(function));
		registry<GhostFunctionRegistry>().add(shared->identifier, shared);
	}

	void Game::traverseData(const std::filesystem::path &dir) {
		for (const auto &entry: std::filesystem::directory_iterator(dir)) {
			if (entry.is_directory()) {
				traverseData(entry.path());
			} else if (entry.is_regular_file()) {
				if (auto path = entry.path(); path.extension() == ".json")
					loadDataFile(path);
			}
		}
	}

	void Game::loadDataFile(const std::filesystem::path &file) {
		std::ifstream ifs(file);
		std::stringstream ss;
		ss << ifs.rdbuf();
		std::string raw = ss.str();
		nlohmann::json json = nlohmann::json::parse(raw);
		Identifier type = json.at(0);

		// TODO: make a map of handlers for different types instead of if-elsing here
		if (type == "base:entity_texture_map"_id) {

			auto &textures = registry<EntityTextureRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				textures.add(Identifier(key), EntityTexture(Identifier(key), value.at(0), value.at(1)));

		} else if (type == "base:ghost_details_map"_id) {

			auto &details = registry<GhostDetailsRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				details.add(Identifier(key), GhostDetails(Identifier(key), value.at(0), value.at(1), value.at(2), value.at(3), value.at(4), value.at(5), value.at(6)));

		} else if (type == "base:item_texture_map"_id) {

			auto &textures = registry<ItemTextureRegistry>();
			for (const auto &[key, value]: json.at(1).items()) {
				if (value.size() == 3)
					textures.add(Identifier(key), ItemTexture(Identifier(key), value.at(0), value.at(1), value.at(2)));
				else if (value.size() == 5)
					textures.add(Identifier(key), ItemTexture(Identifier(key), value.at(0), value.at(1), value.at(2), value.at(3), value.at(4)));
				else
					throw std::invalid_argument("Expected ItemTexture JSON size to be 3 or 5, not " + std::to_string(value.size()));
			}

		} else if (type == "base:ore_map"_id) {

			auto &ores = registry<OreRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				ores.add(Identifier(key), Ore(Identifier(key), ItemStack::fromJSON(*this, value.at(0)), value.at(1), value.at(2), value.at(3), value.at(4), value.at(5)));

		} else if (type == "base:realm_details_map"_id) {

			auto &details = registry<RealmDetailsRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				details.add(Identifier(key), RealmDetails(Identifier(key), value.at("tileset")));

		} else if (type == "base:texture_map"_id) {

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

		} else if (type == "base:tileset_map"_id) {

			auto &tilesets = registry<TilesetRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				tilesets.add(Identifier(key), Tileset::fromJSON(Identifier(key), value));

		} else if (type == "base:recipe_list"_id) {

			for (const auto &recipe_json: json.at(1))
				addRecipe(recipe_json);

		} else if (type == "base:fluid_map"_id) {

			auto &fluids = registry<FluidRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				fluids.add(Identifier(key), Fluid(Identifier(key), value.at("tileset"), value.at("tilename")));

		} else if (type == "base:crop_map"_id) {

			auto &crops = registry<CropRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				crops.add(Identifier(key), Crop(Identifier(key), *this, value));

		} else
			throw std::runtime_error("Unknown data file type: " + type.str());
	}

	void Game::addRecipe(const nlohmann::json &json) {
		registries.at(json.at(0).get<Identifier>())->toUnnamed()->add(*this, json.at(1));
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
		return static_cast<long>(base) % 24 + fractional(base);
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

	std::shared_ptr<Tile> Game::getTile(const Identifier &identifier) {
		auto &reg = registry<TileRegistry>();
		if (auto found = reg.maybe(identifier))
			return found;
		static auto default_tile = std::make_shared<Tile>("base:tile"_id);
		return default_tile;
	}

	GamePtr Game::create(Side side, const ServerArgument &argument) {
		GamePtr out;
		if (side == Side::Client)
			out = GamePtr(new ClientGame(*std::get<Canvas *>(argument)));
		else
			out = GamePtr(new ServerGame(std::get<std::shared_ptr<LocalServer>>(argument)));
		out->initialSetup();
		return out;
	}

	GamePtr Game::fromJSON(Side side, const nlohmann::json &json, const ServerArgument &argument) {
		auto out = create(side, argument);
		out->initialSetup();
		for (const auto &[string, realm_json]: json.at("realms").get<std::unordered_map<std::string, nlohmann::json>>())
			out->realms.emplace(parseUlong(string), Realm::fromJSON(*out, realm_json));
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
		return std::dynamic_pointer_cast<ClientGame>(shared_from_this());
	}

	ServerGame & Game::toServer() {
		return dynamic_cast<ServerGame &>(*this);
	}

	const ServerGame & Game::toServer() const {
		return dynamic_cast<const ServerGame &>(*this);
	}

	std::shared_ptr<ServerGame> Game::toServerPointer() {
		assert(getSide() == Side::Server);
		return std::dynamic_pointer_cast<ServerGame>(shared_from_this());
	}

	void to_json(nlohmann::json &json, const Game &game) {
		json["debugMode"] = game.debugMode;
		json["realms"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[id, realm]: game.realms)
			json["realms"][std::to_string(id)] = nlohmann::json(*realm);
		json["hourOffset"] = game.getHour();
		if (0 < game.cavesGenerated)
			json["cavesGenerated"] = game.cavesGenerated;
	}
}
