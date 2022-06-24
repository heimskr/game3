#include <iostream>

#include "Tiles.h"
#include "entity/Blacksmith.h"
#include "entity/Merchant.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/Stonks.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Teleporter.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/InventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	Blacksmith::Blacksmith(EntityID id_):
		Worker(id_, Entity::BLACKSMITH_TYPE) {}

	Blacksmith::Blacksmith(EntityID id_, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_):
		Worker(id_, Entity::BLACKSMITH_TYPE, overworld_realm, house_realm, house_position, keep_) {}

	std::shared_ptr<Blacksmith> Blacksmith::create(EntityID id, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_) {
		auto out = std::shared_ptr<Blacksmith>(new Blacksmith(id, overworld_realm, house_realm, house_position, keep_));
		out->init();
		return out;
	}

	std::shared_ptr<Blacksmith> Blacksmith::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Blacksmith>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	bool Blacksmith::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &tab = *getRealm()->getGame().canvas.window.inventoryTab;
		std::cout << "Blacksmith: money = " << money << ", phase = " << int(phase) << ", stuck = " << stuck << '\n';
		player->queueForMove([player, &tab](const auto &) {
			tab.resetExternalInventory();
			return true;
		});
		tab.setExternalInventory("Blacksmith", inventory);
		return true;
	}

	void Blacksmith::tick(Game &game, float delta) {
		Worker::tick(game, delta);

		if (stillStuck(delta))
			return;

		const auto hour = game.getHour();
		
		if (phase == 0 && WORK_START_HOUR <= hour)
			wakeUp();

		else if (phase == 1 && realmID == overworldRealm)
			goToKeep(2);

		else if (phase == 2 && position == destination)
			goToStockpile(3);

		else if (phase == 3 && position == destination)
			buyResources();

		else if (phase == 4 && BUYING_TIME <= (buyTime += delta))
			leaveKeep(5);

		else if (phase == 5 && realmID == overworldRealm)
			goToHouse(6);

		else if (phase == 6 && position == destination)
			goToBed(7);
	}

	void Blacksmith::wakeUp() {
		const ItemCount iron_bars = inventory->count(Item::IRON_BAR);
		const ItemCount gold_bars = inventory->count(Item::GOLD_BAR);
		const ItemCount diamonds  = inventory->count(Item::DIAMOND);
		const ItemCount coal      = inventory->count(Item::COAL);

		ironOreNeeded = RESOURCE_TARGET - std::min(RESOURCE_TARGET, iron_bars);
		goldOreNeeded = RESOURCE_TARGET - std::min(RESOURCE_TARGET, gold_bars);
		diamondOreNeeded = RESOURCE_TARGET - std::min(RESOURCE_TARGET, diamonds);

		coalNeeded = ironOreNeeded + 2 * goldOreNeeded;
		if (coal < coalNeeded)
			coalNeeded -= coal;
		else
			coalNeeded = 0;

		Game  &game  = getRealm()->getGame();
		Realm &house = *game.realms.at(houseRealm);

		if (0 < coalNeeded || diamonds < RESOURCE_TARGET) {
			phase = 1;
			pathfind(house.getTileEntity<Teleporter>()->position);
		} else {
			phase = 8;
			pathfind(destination = house.extraData.at("furnace"));
		}
	}
	
	void Blacksmith::buyResources() {
		auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());

		buyTime = 0.f;
		phase = 4;

		const ItemCount iron_ore    = std::min(keep_realm.stockpileInventory->count(Item::IRON_ORE), ironOreNeeded);
		const ItemCount gold_ore    = std::min(keep_realm.stockpileInventory->count(Item::GOLD_ORE), goldOreNeeded);
		const ItemCount diamond_ore = std::min(keep_realm.stockpileInventory->count(Item::GOLD_ORE), diamondOreNeeded);
		const ItemCount coal        = std::min(keep_realm.stockpileInventory->count(Item::COAL),     coalNeeded);

		std::array<ItemStack, 4> stacks {
			ItemStack(Item::IRON_ORE, iron_ore),
			ItemStack(Item::GOLD_ORE, gold_ore),
			ItemStack(Item::GOLD_ORE, diamond_ore),
			ItemStack(Item::COAL,     coal),
		};

		MoneyCount new_money = money;

		for (ItemStack &stack: stacks) {
			MoneyCount buy_price = 0;
			while (0 < stack.count && new_money < (buy_price = totalBuyPrice(keep_realm, stack)))
				--stack.count;
			if (stack.count == 0)
				continue;
			auto leftover = inventory->add(stack);
			if (leftover) {
				stack.count -= leftover->count;
				if (new_money < totalBuyPrice(keep_realm, stack))
					throw std::runtime_error("Buy price calculation failed after reducing stack");
				new_money -= buy_price;
				keep_realm.money += buy_price;
				if (stack.count != keep_realm.stockpileInventory->remove(stack))
					throw std::runtime_error("Couldn't remove enough resources from the stockpile");
				break;
			} else {
				new_money -= buy_price;
				keep_realm.money += buy_price;
				if (stack.count != keep_realm.stockpileInventory->remove(stack))
					throw std::runtime_error("Couldn't remove enough resources from the stockpile");
			}
		}

		setMoney(new_money);
	}
}
