#include <iostream>

#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/Inventory.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/TextTab.h"
#include "util/AStar.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	void Game::initEntities() {
		for (const auto &[realm_id, realm]: realms)
			realm->initEntities();
	}

	void Game::initRecipes() {
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::IRON_BAR, 8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::IRON_PICKAXE),    CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::IRON_BAR, 6}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::IRON_SHOVEL),     CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::IRON_BAR, 8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::IRON_AXE),        CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::IRON_BAR, 8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::IRON_HAMMER),     CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::GOLD_BAR, 8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::GOLD_PICKAXE),    CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::GOLD_BAR, 6}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::GOLD_SHOVEL),     CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::GOLD_BAR, 8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::GOLD_AXE),        CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::GOLD_BAR, 8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::GOLD_HAMMER),     CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::DIAMOND,  8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::DIAMOND_PICKAXE), CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::DIAMOND,  6}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::DIAMOND_SHOVEL),  CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::DIAMOND,  8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::DIAMOND_AXE),     CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::DIAMOND,  8}, {Item::WOOD, 4}}, ItemStack::withDurability(Item::DIAMOND_HAMMER),  CraftingStationType::Anvil);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::DIAMOND_ORE, 1}}, ItemStack(Item::DIAMOND, 1), CraftingStationType::Anvil);

		registerPrimaryRecipe(std::vector<ItemStack> {{Item::IRON_ORE, 1}, {Item::COAL, 1}}, ItemStack(Item::IRON_BAR, 1), CraftingStationType::Furnace);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::GOLD_ORE, 1}, {Item::COAL, 2}}, ItemStack(Item::GOLD_BAR, 1), CraftingStationType::Furnace);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::CLAY,  1}}, ItemStack(Item::BRICK, 1), CraftingStationType::Furnace);
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::CLAY, 10}}, ItemStack(Item::POT,   1), CraftingStationType::Furnace);

		registerPrimaryRecipe(std::vector<ItemStack> {{Item::POT, 1}, {Item::SAPLING, 1}}, ItemStack(Item::PLANT_POT1, 1));
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::POT, 1}, {Item::SAPLING, 1}}, ItemStack(Item::PLANT_POT2, 1));
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::POT, 1}, {Item::SAPLING, 1}}, ItemStack(Item::PLANT_POT3, 1));
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::STONE, 8}}, ItemStack(Item::TOWER, 1));
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::STONE, 10}, {Item::PLANK, 10}}, ItemStack(Item::CAVE_ENTRANCE, 1));
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::IRON_BAR, 10}}, ItemStack(Item::CAULDRON, 1));

		// Temporary recipes
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::WOOD,  1}}, ItemStack(Item::PLANK, 1));
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::PLANK, 2}}, ItemStack(Item::WOODEN_WALL, 1));
		registerPrimaryRecipe(std::vector<ItemStack> {{Item::STONE, 1}}, ItemStack(Item::BOMB, 64));
	}

	void Game::initInteractionSets() {
		interactionSets.clear();
		auto standard = std::make_shared<StandardInteractions>();
		for (const RealmType type: Realm::allTypes)
			interactionSets.emplace(type, standard);
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
			out->realms.emplace(parseUlong(string), Realm::fromJSON(realm_json)).first->second->setGame(*out);
		out->activeRealm = out->realms.at(json.at("activeRealmID"));
		out->hourOffset = json.contains("hourOffset")? json.at("hourOffset").get<float>() : 0.f;
		out->debugMode = json.contains("debugMode")? json.at("debugMode").get<bool>() : false;
		out->cavesGenerated = json.contains("cavesGenerated")? json.at("cavesGenerated").get<decltype(Game::cavesGenerated)>() : 0;
		return out;
	}

	void Game::registerPrimaryRecipe(std::vector<ItemStack> &&inputs, ItemStack &&output, CraftingStationType station) {
		const ItemID id = output.item->id;
		CraftingRecipe recipe(std::move(inputs), std::move(output), station);
		recipes.push_back(recipe);
		primaryRecipes.emplace(id, std::move(recipe));
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
