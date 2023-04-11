#include <fstream>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "entity/Blacksmith.h"
#include "entity/EntityFactory.h"
#include "entity/ItemEntity.h"
#include "entity/Merchant.h"
#include "entity/Miner.h"
#include "entity/Player.h"
#include "entity/Woodcutter.h"
#include "entity/Worker.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "item/Bomb.h"
#include "item/CaveEntrance.h"
#include "item/Furniture.h"
#include "item/Hammer.h"
#include "item/Item.h"
#include "item/Landfill.h"
#include "item/Landfills.h"
#include "item/Mushroom.h"
#include "item/Sapling.h"
#include "item/Tool.h"
#include "realm/Keep.h"
#include "realm/RealmFactory.h"
#include "recipe/CraftingRecipe.h"
#include "registry/Registries.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Ghost.h"
#include "tileentity/ItemSpawner.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Sign.h"
#include "tileentity/Stockpile.h"
#include "tileentity/Teleporter.h"
#include "tileentity/TileEntity.h"
#include "tileentity/TileEntityFactory.h"
#include "tileentity/Tree.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"
#include "util/AStar.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	void Game::initRegistries() {
		registries.clear();
		registries.add<CraftingRecipeRegistry>();
		registries.add<ItemRegistry>();
		registries.add<DurabilityRegistry>();
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
		// TODO: plugins
	}

	void Game::addItems() {
		add(std::make_shared<Item>        ("base:item/shortsword",      "Shortsword",      100,  1));
		add(std::make_shared<Item>        ("base:item/red_potion",      "Red Potion",       20,  8));
		add(std::make_shared<Item>        ("base:item/coins",           "Gold",              1, 1'000'000));
		add(std::make_shared<Item>        ("base:item/iron_ore",        "Iron Ore",         10, 64));
		add(std::make_shared<Item>        ("base:item/copper_ore",      "Copper Ore",        8, 64));
		add(std::make_shared<Item>        ("base:item/gold_ore",        "Gold Ore",         20, 64));
		add(std::make_shared<Item>        ("base:item/diamond_ore",     "Diamond Ore",      80, 64));
		add(std::make_shared<Item>        ("base:item/diamond",         "Diamond",         100, 64));
		add(std::make_shared<Item>        ("base:item/coal",            "Coal",              5, 64));
		add(std::make_shared<Item>        ("base:item/oil",             "Oil",              15, 64));
		add(std::make_shared<Item>        ("base:item/wood",            "Wood",              3, 64));
		add(std::make_shared<Item>        ("base:item/stone",           "Stone",             1, 64));
		add(std::make_shared<Item>        ("base:item/iron_bar",        "Iron Bar",         16, 64));
		add(std::make_shared<Sapling>     ("base:item/sapling",         "Sapling",           5, 64));
		add(std::make_shared<Item>        ("base:item/gold_bar",        "Gold Bar",         45, 64));
		add(std::make_shared<Furniture>   ("base:item/wooden_wall",     "Wooden Wall",       9, 64));
		add(std::make_shared<Item>        ("base:item/plank",           "Plank",             4, 64));
		add(std::make_shared<Item>        ("base:item/dirt",            "Dirt",              1, 64));
		add(std::make_shared<Item>        ("base:item/brick",           "Brick",             3, 64));
		add(std::make_shared<Item>        ("base:item/pot",             "Pot",              24, 64));
		add(std::make_shared<Furniture>   ("base:item/plant_pot1",      "Plant Pot",        32, 64));
		add(std::make_shared<Furniture>   ("base:item/plant_pot2",      "Plant Pot",        32, 64));
		add(std::make_shared<Furniture>   ("base:item/plant_pot3",      "Plant Pot",        32, 64));
		add(std::make_shared<Furniture>   ("base:item/tower",           "Tower",            10, 64));
		add(std::make_shared<CaveEntrance>("base:item/cave_entrance",   "Cave Entrance",    50,  1));
		add(std::make_shared<Item>        ("base:item/mead",            "Mead",             10, 16));
		add(std::make_shared<Item>        ("base:item/honey",           "Honey",             5, 64));
		add(std::make_shared<Bomb>        ("base:item/bomb",            "Bomb",             32, 64));
		add(std::make_shared<Item>        ("base:item/ash",             "Ash",               1, 64));
		add(std::make_shared<Item>        ("base:item/silicon",         "Silicon",           2, 64));
		add(std::make_shared<Item>        ("base:item/electronics",     "Electronics",      32, 64));
		add(std::make_shared<Item>        ("base:item/sulfur",          "Sulfur",           15, 64));
		add(std::make_shared<Furniture>   ("base:item/cauldron",        "Cauldron",        175,  1));
		add(std::make_shared<Furniture>   ("base:item/purifier",        "Purifier",        300,  1));
		add(std::make_shared<Hammer>      ("base:item/iron_hammer",     "Iron Hammer",     150,  3.f, 128));
		add(std::make_shared<Hammer>      ("base:item/gold_hammer",     "Gold Hammer",     400, .75f, 128));
		add(std::make_shared<Hammer>      ("base:item/diamond_hammer",  "Diamond Hammer",  900,  1.f, 128));
		add(std::make_shared<Tool>        ("base:item/iron_axe",        "Iron Axe",        150,  3.f, 128, ItemAttribute::Axe));
		add(std::make_shared<Tool>        ("base:item/iron_pickaxe",    "Iron Pickaxe",    150,  3.f,  64, ItemAttribute::Pickaxe));
		add(std::make_shared<Tool>        ("base:item/iron_shovel",     "Iron Shovel",     120,  3.f,  64, ItemAttribute::Shovel));
		add(std::make_shared<Tool>        ("base:item/gold_axe",        "Gold Axe",        400, .75f,  64, ItemAttribute::Axe));
		add(std::make_shared<Tool>        ("base:item/gold_pickaxe",    "Gold Pickaxe",    400, .75f,  64, ItemAttribute::Pickaxe));
		add(std::make_shared<Tool>        ("base:item/gold_shovel",     "Gold Shovel",     300, .75f, 512, ItemAttribute::Shovel));
		add(std::make_shared<Tool>        ("base:item/diamond_axe",     "Diamond Axe",     900,  1.f, 512, ItemAttribute::Axe));
		add(std::make_shared<Tool>        ("base:item/diamond_pickaxe", "Diamond Pickaxe", 900,  1.f, 512, ItemAttribute::Pickaxe));
		add(std::make_shared<Tool>        ("base:item/diamond_shovel",  "Diamond Shovel",  700,  1.f, 512, ItemAttribute::Shovel));
		add(std::make_shared<Landfill>    ("base:item/sand",            "Sand",              1, 64, "base:tileset/monomap", "base:tile/shallow_water", Landfill::DEFAULT_COUNT, "base:tile/sand"));
		add(std::make_shared<Landfill>    ("base:item/volcanic_sand",   "Volcanic Sand",     3, 64, "base:tileset/monomap", "base:tile/shallow_water", Landfill::DEFAULT_COUNT, "base:tile/volcanic_sand"));
		add(std::make_shared<Landfill>    ("base:item/clay",            "Clay",              2, 64, clayRequirement));
		add(std::make_shared<Mushroom>("base:item/saffron_milkcap", "Saffron Milkcap",    10, 1 ));
		add(std::make_shared<Mushroom>("base:item/honey_fungus",    "Honey Fungus",       15, 18));
		add(std::make_shared<Mushroom>("base:item/brittlegill",     "Golden Brittlegill", 20, 7 ));
		add(std::make_shared<Mushroom>("base:item/indigo_milkcap",  "Indigo Milkcap",     20, 11));
		add(std::make_shared<Mushroom>("base:item/black_trumpet",   "Black Trumpet",      20, 29));
		add(std::make_shared<Mushroom>("base:item/grey_knight",     "Grey Knight",        20, 12));
	}

	void Game::addGhosts() {
		Game3::initGhosts(*this);
	}

	void Game::addEntityFactories() {
		add(EntityFactory::create<Blacksmith>());
		add(EntityFactory::create<ItemEntity>());
		add(EntityFactory::create<Merchant>());
		add(EntityFactory::create<Miner>());
		add(EntityFactory::create<Player>());
		add(EntityFactory::create<Woodcutter>());
		add(EntityFactory::create<Worker>()); // TODO: verify whether adding this base class is necessary
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
	}

	void Game::addRealms() {
		auto &types = registry<RealmTypeRegistry>();
		auto &factories = registry<RealmFactoryRegistry>();

		auto addRealm = [&]<typename T>(const Identifier &id) {
			types.add(id);
			factories.add(id, std::make_shared<RealmFactory>(RealmFactory::create<T>(id)));
		};

		// ...
		addRealm.operator()<Realm>("base:realm/overworld"_id);
		addRealm.operator()<Realm>("base:realm/house"_id);
		addRealm.operator()<Realm>("base:realm/blacksmith"_id);
		addRealm.operator()<Realm>("base:realm/cave"_id);
		addRealm.operator()<Realm>("base:realm/tavern"_id);
		addRealm.operator()<Keep>(Keep::ID());
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
		if (type == "base:durability_map"_id) {

			auto &durabilities = registry<DurabilityRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				durabilities.add(Identifier(key), NamedDurability(Identifier(key), value.get<Durability>()));

		} else if (type == "base:entity_texture_map"_id) {

			auto &textures = registry<EntityTextureRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				textures.add(Identifier(key), EntityTexture(Identifier(key), value.at(0), value.at(1)));

		} else if (type == "base:ghost_details_map"_id) {

			auto &details = registry<GhostDetailsRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				details.add(Identifier(key), GhostDetails(Identifier(key), value.at(0), value.at(1), value.at(2), value.at(3), value.at(4), value.at(5)));

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
				ores.add(Identifier(key), Ore(Identifier(key), ItemStack::fromJSON(*this, json.at(0)), json.at(1), json.at(2), json.at(3), json.at(4), json.at(5)));

		} else if (type == "base:realm_details_map"_id) {

			auto &details = registry<RealmDetailsRegistry>();
			for (const auto &[key, value]: json.at(1).items())
				details.add(Identifier(key), RealmDetails(Identifier(key), value.at("tileset")));

		} else if (type == "base:texture_map"_id) {

			auto &textures = registry<TextureRegistry>();
			for (const auto &[key, value]: json.at(1).items()) {
				if (value.size() == 1)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0)));
				else if (value.size() == 2)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0), value.at(1)));
				else if (value.size() == 3)
					textures.add(Identifier(key), Texture(Identifier(key), value.at(0), value.at(1), value.at(2)));
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

		} else
			throw std::runtime_error("Unknown data file type: " + type.str());
	}

	void Game::addRecipe(const nlohmann::json &json) {
		registries.at(json.at(0).get<Identifier>())->toUnnamed()->add(*this, json.at(1));
	}

	void Game::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1'000'000'000.f;
		for (auto &[id, realm]: realms)
			realm->tick(delta);
		player->ticked = false;
	}

	RealmID Game::newRealmID() const {
		// TODO: a less stupid way of doing this.
		RealmID max = 1;
		for (const auto &[id, realm]: realms)
			max = std::max(max, id);
		return max + 1;
	}

	void Game::setText(const Glib::ustring &text, const Glib::ustring &name, bool focus, bool ephemeral) {
		if (canvas.window.textTab) {
			auto &tab = *canvas.window.textTab;
			tab.text = text;
			tab.name = name;
			tab.ephemeral = ephemeral;
			if (focus)
				tab.show();
			tab.reset(shared_from_this());
		}
	}

	const Glib::ustring & Game::getText() const {
		if (canvas.window.textTab)
			return canvas.window.textTab->text;
		throw std::runtime_error("Can't get text: TextTab is null");
	}

	void Game::click(int button, int, double pos_x, double pos_y) {
		if (!activeRealm)
			return;

		auto &realm = *activeRealm;
		const auto width  = canvas.width();
		const auto height = canvas.height();

		if (0 < realm.ghostCount && width - 42.f <= pos_x && pos_x < width - 10.f && height - 42.f <= pos_y && pos_y < height - 10.f) {
			realm.confirmGhosts();
			return;
		}

		const auto [x, y] = translateCanvasCoordinates(pos_x, pos_y);

		if (button == 1) {
			if (auto *stack = player->inventory->getActive())
				stack->item->use(player->inventory->activeSlot, *stack, {{y, x}, activeRealm, player});
		} else if (button == 3 && player && realm.isValid({y, x}) && !realm.rightClick({y, x}, pos_x, pos_y) && debugMode) {
			player->teleport({y, x});
		}
	}

	Position Game::translateCanvasCoordinates(double x, double y) const {
		const auto &realm   = *activeRealm;
		const auto &tilemap = realm.tilemap1;
		const auto scale    = canvas.scale;
		x -= canvas.width() / 2.f - (tilemap->width * tilemap->tileSize / 4.f) * scale + canvas.center.x() * canvas.magic * scale;
		x /= tilemap->tileSize * scale / 2.f;
		y -= canvas.height() / 2.f - (tilemap->height * tilemap->tileSize / 4.f) * scale + canvas.center.y() * canvas.magic * scale;
		y /= tilemap->tileSize * scale / 2.f;
		return {static_cast<Index>(x), static_cast<Index>(y)};
	}

	Gdk::Rectangle Game::getVisibleRealmBounds() const {
		const auto [left,     top] = translateCanvasCoordinates(0., 0.);
		const auto [right, bottom] = translateCanvasCoordinates(canvas.width(), canvas.height());
		return {left, top, right - left + 1, bottom - top + 1};
	}

	double Game::getTotalSeconds() const {
		return std::chrono::duration_cast<std::chrono::nanoseconds>(getTime() - startTime).count() / 1e9;
	}

	double Game::getHour() const {
		const auto base = getTotalSeconds() / 10. + hourOffset;
		return static_cast<long>(base) % 24 + fractional(base);
	}

	double Game::getMinute() const {
		return 60. * fractional(getHour());
	}

	double Game::getDivisor() const {
		return 3. - 2. * sin(getHour() * 3.1415926 / 24.);
	}

	void Game::activateContext() {
		canvas.window.activateContext();
	}

	MainWindow & Game::getWindow() {
		return canvas.window;
	}

	GamePtr Game::create(Canvas &canvas) {
		return GamePtr(new Game(canvas));
	}

	GamePtr Game::fromJSON(const nlohmann::json &json, Canvas &canvas) {
		auto out = create(canvas);
		for (const auto &[string, realm_json]: json.at("realms").get<std::unordered_map<std::string, nlohmann::json>>())
			out->realms.emplace(parseUlong(string), Realm::fromJSON(*out, realm_json));
		out->activeRealm = out->realms.at(json.at("activeRealmID"));
		out->hourOffset = json.contains("hourOffset")? json.at("hourOffset").get<float>() : 0.f;
		out->debugMode = json.contains("debugMode")? json.at("debugMode").get<bool>() : false;
		out->cavesGenerated = json.contains("cavesGenerated")? json.at("cavesGenerated").get<decltype(Game::cavesGenerated)>() : 0;
		return out;
	}

	void to_json(nlohmann::json &json, const Game &game) {
		json["activeRealmID"] = game.activeRealm->id;
		json["debugMode"] = game.debugMode;
		json["realms"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[id, realm]: game.realms)
			json["realms"][std::to_string(id)] = nlohmann::json(*realm);
		json["hourOffset"] = game.getHour();
		if (0 < game.cavesGenerated)
			json["cavesGenerated"] = game.cavesGenerated;
	}
}
