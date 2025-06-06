#include "algorithm/Stonks.h"
#include "entity/Blacksmith.h"
#include "entity/Merchant.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Teleporter.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	Blacksmith::Blacksmith():
		Entity(ID()), Worker(ID()), Merchant(ID()) {}

	Blacksmith::Blacksmith(RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_):
		Entity(ID()), Worker(ID(), overworld_realm, house_realm, house_position, std::move(keep_)), Merchant(ID()) {}

	std::shared_ptr<Blacksmith> Blacksmith::create(const std::shared_ptr<Game> &) {
		return Entity::create<Blacksmith>();
	}

	std::shared_ptr<Blacksmith> Blacksmith::create(const std::shared_ptr<Game> &, RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_) {
		return Entity::create<Blacksmith>(overworld_realm, house_realm, house_position, std::move(keep_));
	}

	std::shared_ptr<Blacksmith> Blacksmith::fromJSON(const GamePtr &game, const boost::json::value &json) {
		auto out = Entity::create<Blacksmith>();
		out->absorbJSON(game, json);
		return out;
	}

	void Blacksmith::toJSON(boost::json::value &json) const {
		Entity::toJSON(json);
		Worker::toJSON(json);
		Merchant::toJSON(json);
		if (0.f < actionTime) {
			json.as_object()["actionTime"] = actionTime;
		}
	}

	void Blacksmith::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		Entity::absorbJSON(game, json);
		Worker::absorbJSON(game, json);
		Merchant::absorbJSON(game, json);
		const auto &object = json.as_object();
		if (auto iter = object.find("actionTime"); iter != object.end()) {
			actionTime = getDouble(iter->value());
		}
	}

	bool Blacksmith::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		std::cout << "Blacksmith: money = " << money << ", phase = " << int(phase) << ", stuck = " << stuck << '\n';

		if (phase != 10) {
			player->showText("Sorry, I'm not selling anything right now.", "Blacksmith");
		} else if (getSide() == Side::Client) {
			// auto &window = getRealm()->getGame().toClient().getWindow();
			// auto &tab    = *window.merchantTab;
			// player->queueForMove([&tab](const auto &) {
			// 	tab.hide();
			// 	return true;
			// });
			// tab.show();
			// window.delay([this, &tab] {
			// 	tab.setMerchantInventory("Blacksmith", std::dynamic_pointer_cast<ClientInventory>(inventory), greed);
			// }, 2);
		}

		return true;
	}

	void Blacksmith::tick(const TickArgs &args) {
		Worker::tick(args);

		const auto delta = args.delta;

		if (args.game->getSide() == Side::Client || stillStuck(delta))
			return;

		const auto hour = args.game->getHour();

		if (phase == 0 && WORK_START_HOUR <= hour)
			wakeUp();

		else if (phase == 1 && realmID == overworldRealm)
			goToKeep(2);

		else if (phase == 2 && position == destination)
			goToStockpile(3);

		else if (phase == 3 && position == destination)
			buyResources();

		else if (phase == 4) {
			actionTime += delta;
			if (BUYING_TIME <= actionTime)
				leaveKeep(5);
		}

		else if (phase == 5 && realmID == overworldRealm)
			goToHouse(6);

		else if (phase == 6 && position == destination)
			goToForge();

		else if (phase == 7 && position == destination)
			craftTools();

		else if (phase == 8) {
			actionTime += delta;
			if (CRAFTING_TIME <= actionTime)
				goToCounter();
		}

		else if (phase == 9 && position == destination)
			startSelling();

		else if (phase == 10 && WORK_END_HOUR <= hour)
			goToBed(11);

		else if (phase == 11 && position == destination)
			phase = 12;

		else if (phase == 12 && hour < WORK_END_HOUR)
			phase = 0;
	}

	void Blacksmith::encode(Buffer &buffer) {
		Entity::encode(buffer);
		Worker::encode(buffer);
		Merchant::encode(buffer);
		buffer << actionTime;
		buffer << coalNeeded;
		buffer << ironOreNeeded;
		buffer << goldOreNeeded;
		buffer << diamondOreNeeded;
	}

	void Blacksmith::decode(Buffer &buffer) {
		Entity::decode(buffer);
		Worker::decode(buffer);
		Merchant::decode(buffer);
		buffer >> actionTime;
		buffer >> coalNeeded;
		buffer >> ironOreNeeded;
		buffer >> goldOreNeeded;
		buffer >> diamondOreNeeded;
	}

	void Blacksmith::wakeUp() {
		const InventoryPtr inventory = getInventory(0);
		const ItemCount iron_bars = inventory->count(Identifier("base", "item/iron_bar"));
		const ItemCount gold_bars = inventory->count(Identifier("base", "item/gold_bar"));
		const ItemCount diamonds  = inventory->count(Identifier("base", "item/diamond"));
		const ItemCount coal      = inventory->count(Identifier("base", "item/coal"));

		ironOreNeeded = RESOURCE_TARGET - std::min(RESOURCE_TARGET, iron_bars);
		goldOreNeeded = RESOURCE_TARGET - std::min(RESOURCE_TARGET, gold_bars);
		diamondOreNeeded = RESOURCE_TARGET - std::min(RESOURCE_TARGET, diamonds);

		coalNeeded = ironOreNeeded + 2 * goldOreNeeded;
		if (coal < coalNeeded)
			coalNeeded -= coal;
		else
			coalNeeded = 0;

		GamePtr game = getRealm()->getGame();
		RealmPtr house = game->getRealm(houseRealm);

		if (0 < coalNeeded || diamonds < RESOURCE_TARGET) {
			phase = 1;
			pathfind(house->getTileEntity<Teleporter>()->position);
		} else {
			phase = 8;
			destination = boost::json::value_to<Position>(house->extraData.at("furnace"));
			pathfind(destination);
		}
	}

	void Blacksmith::buyResources() {
		// auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());

		// actionTime = 0.f;
		// phase = 4;

		// const ItemCount iron_ore    = std::min(keep_realm.stockpileInventory->count(Identifier("base", "item/iron_ore")), ironOreNeeded);
		// const ItemCount gold_ore    = std::min(keep_realm.stockpileInventory->count(Identifier("base", "item/gold_ore")), goldOreNeeded);
		// const ItemCount diamond_ore = std::min(keep_realm.stockpileInventory->count(Identifier("base", "item/gold_ore")), diamondOreNeeded);
		// const ItemCount coal        = std::min(keep_realm.stockpileInventory->count(Identifier("base", "item/coal")),     coalNeeded);

		// Game &game = getGame();
		// std::array<ItemStack, 4> stacks {
		// 	ItemStack(game, "base:item/iron_ore", iron_ore),
		// 	ItemStack(game, "base:item/gold_ore", gold_ore),
		// 	ItemStack(game, "base:item/gold_ore", diamond_ore),
		// 	ItemStack(game, "base:item/coal",     coal),
		// };

		// MoneyCount new_money = money;

		// for (ItemStack &stack: stacks) {
		// 	MoneyCount buy_price = 0;
		// 	while (0 < stack.count && new_money < (buy_price = totalBuyPrice(keep_realm, stack)))
		// 		--stack.count;
		// 	if (stack.count == 0)
		// 		continue;
		// 	auto leftover = getInventory(0)->add(stack);
		// 	if (leftover) {
		// 		stack.count -= leftover->count;
		// 		if (new_money < totalBuyPrice(keep_realm, stack))
		// 			throw std::runtime_error("Buy price calculation failed after reducing stack");
		// 		new_money -= buy_price;
		// 		keep_realm.money += buy_price;
		// 		if (stack.count != keep_realm.stockpileInventory->remove(stack))
		// 			throw std::runtime_error("Couldn't remove enough resources from the stockpile");
		// 		break;
		// 	} else {
		// 		new_money -= buy_price;
		// 		keep_realm.money += buy_price;
		// 		if (stack.count != keep_realm.stockpileInventory->remove(stack))
		// 			throw std::runtime_error("Couldn't remove enough resources from the stockpile");
		// 	}
		// }

		// setMoney(new_money);
	}

	void Blacksmith::goToForge() {
		auto house = std::dynamic_pointer_cast<Building>(getRealm()->tileEntityAt(housePosition));
		if (!house)
			throw std::runtime_error("Blacksmith couldn't find house");
		house->teleport(getSelf());

		auto realm = getRealm();
		if (realm->id != houseRealm) {
			// throw std::runtime_error("Blacksmith couldn't teleport to house");
			stuck = true;
			return;
		}

		destination = boost::json::value_to<Position>(realm->extraData.at("furnace")) + Position(1, 0);

		if (!pathfind(destination)) {
			// throw std::runtime_error("Blacksmith couldn't pathfind to forge");
			stuck = true;
			return;
		}

		phase = 7;
	}

	void Blacksmith::craftTools() {
		// TODO: resurrect
		/*
		Game &game = getRealm()->getGame();

		phase = 8;
		actionTime = 0.f;

		static std::vector<std::pair<ItemID, ItemCount>> tools {
			{Item::IRON_AXE,    1}, {Item::IRON_SHOVEL,    1}, {Item::IRON_PICKAXE,    3}, {Item::IRON_HAMMER,    1},
			{Item::GOLD_AXE,    1}, {Item::GOLD_SHOVEL,    1}, {Item::GOLD_PICKAXE,    3}, {Item::GOLD_HAMMER,    1},
			{Item::DIAMOND_AXE, 1}, {Item::DIAMOND_SHOVEL, 1}, {Item::DIAMOND_PICKAXE, 3}, {Item::DIAMOND_HAMMER, 1},
		};

		for (auto [item, count]: tools)
			for (size_t i = inventory->count(item); i < count; ++i) {
				std::vector<ItemStack> leftovers;
				// std::cout << "Crafting " << ItemStack(item) << '\n';
				if (!inventory->craft(game.primaryRecipes.at(item), leftovers)) {
					// std::cout << "Couldn't craft. Breaking.\n";
					break;
				}
				if (!leftovers.empty()) {
					// std::cout << "There were leftovers. Returning.\n";
					return;
				}
			}
		// std::cout << "Done.\n";
		//*/
	}

	void Blacksmith::goToCounter() {
		destination = boost::json::value_to<Position>(getRealm()->extraData.at("counter"));
		if (!pathfind(destination)) {
			stuck = true;
		} else {
			phase = 9;
		}
	}

	void Blacksmith::startSelling() {
		phase = 10;
	}
}
