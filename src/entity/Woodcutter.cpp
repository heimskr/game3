#include <iostream>

#include "threading/ThreadContext.h"
#include "Tileset.h"
#include "entity/Woodcutter.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/Stonks.h"
#include "net/Buffer.h"
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
	Woodcutter::Woodcutter():
		Entity(ID()),
		Worker(ID()) {}

	Woodcutter::Woodcutter(RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_):
		Entity(ID()),
		Worker(ID(), overworld_realm, house_realm, std::move(house_position), std::move(keep_)) {}

	std::shared_ptr<Woodcutter> Woodcutter::create(Game &) {
		return Entity::create<Woodcutter>();
	}

	std::shared_ptr<Woodcutter> Woodcutter::create(Game &, RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_) {
		return Entity::create<Woodcutter>(overworld_realm, house_realm, std::move(house_position), std::move(keep_));
	}

	std::shared_ptr<Woodcutter> Woodcutter::fromJSON(Game &game, const nlohmann::json &json) {
		auto out = Entity::create<Woodcutter>();
		out->absorbJSON(game, json);
		return out;
	}

	void Woodcutter::toJSON(nlohmann::json &json) const {
		Worker::toJSON(json);
		json["harvestingTime"] = harvestingTime;
		if (chosenResource)
			json["chosenResource"] = *chosenResource;
	}

	void Woodcutter::absorbJSON(Game &game, const nlohmann::json &json) {
		Worker::absorbJSON(game, json);
		harvestingTime = json.at("harvestingTime");
		if (json.contains("chosenResource"))
			chosenResource = json.at("chosenResource");
	}

	bool Woodcutter::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers) {
		if (getSide() == Side::Client) {
			auto &window = getRealm()->getGame().toClient().getWindow();
			auto &tab = *window.inventoryTab;
			std::cout << "Woodcutter: money = " << money << ", phase = " << int(phase) << ", stuck = " << stuck << '\n';
			player->queueForMove([player, &tab](const auto &) {
				tab.removeModule();
				return true;
			});
			window.showExternalInventory(std::dynamic_pointer_cast<ClientInventory>(inventory));
		}
		return true;
	}

	void Woodcutter::tick(Game &game, float delta) {
		Worker::tick(game, delta);

		if (getSide() == Side::Client || stillStuck(delta))
			return;

		const auto hour = game.getHour();

		if (phase == 0 && WORK_START_HOUR <= hour)
			wakeUp();

		else if (phase == 1 && realmID == overworldRealm)
			goToResource();

		else if (phase == 2 && position == destination)
			startHarvesting();

		else if (phase == 3) {
			harvest(delta);
			if (WORK_END_HOUR <= hour)
				phase = 4;
		}

		else if (phase == 4)
			goToKeep(5);

		else if (phase == 5 && position == destination)
			goToStockpile(6);

		else if (phase == 6 && position == destination)
			sellInventory();

		else if (phase == 7) {
			sellTime += delta;
			if (SELLING_TIME <= sellTime) {
				sellTime = 0;
				leaveKeep(8);
			}
		}

		else if (phase == 8 && realmID == overworldRealm)
			goToHouse(9);

		else if (phase == 9 && position == destination)
			goToBed(10);

		else if (phase == 10 && position == destination)
			phase = 11;

		else if (phase == 11 && hour < WORK_END_HOUR)
			phase = 0;
	}

	void Woodcutter::encode(Buffer &buffer) {
		Worker::encode(buffer);
		buffer << chosenResource << harvestingTime;
	}

	void Woodcutter::decode(Buffer &buffer) {
		Worker::decode(buffer);
		buffer >> chosenResource << harvestingTime;
	}

	void Woodcutter::wakeUp() {
		phase = 1;
		auto &game = getRealm()->getGame();
		auto overworld = game.getRealm(overworldRealm);
		auto house     = game.getRealm(houseRealm);
		// Detect all resources within a given radius of the house
		std::vector<Position> resource_choices;
		for (const auto &[te_position, tile_entity]: overworld->tileEntities)
			if (dynamic_cast<OreDeposit *>(tile_entity.get()))
				resource_choices.push_back(te_position);
		// If there are no resources, get stuck forever. Seed -1998 has no resources.
		if (resource_choices.empty()) {
			phase = -1;
			return;
		}
		// Choose one at random
		chosenResource = choose(resource_choices, threadContext.rng);
		// Pathfind to the door
		pathfind(house->getTileEntity<Teleporter>()->position);
	}

	void Woodcutter::goToResource() {
		auto realm = getRealm();
		if (auto next = realm->getPathableAdjacent(*chosenResource)) {
			if (!pathfind(destination = *next)) {
				stuck = true;
				return;
			}
			setPhase(2);
		} else
			stuck = true;
	}

	void Woodcutter::startHarvesting() {
		setPhase(3);
		harvestingTime = 0.f;
	}

	void Woodcutter::harvest(float delta) {
		if (HARVESTING_TIME <= harvestingTime) {
			harvestingTime = 0.f;
			auto realm = getRealm();
			auto &deposit = dynamic_cast<OreDeposit &>(*realm->tileEntityAt(*chosenResource));
			const ItemStack stack = deposit.getOre(getGame()).stack;
			const auto leftover = inventory->add(stack);
			if (leftover == stack)
				setPhase(4);
		} else
			harvestingTime += delta;
	}

	void Woodcutter::sellInventory() {
		phase = 7;
		auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());
		MoneyCount new_money = money;

		for (Slot slot = 0; slot < inventory->slotCount; ++slot) {
			if (!inventory->contains(slot))
				continue;

			ItemStack stack = inventory->front();
			MoneyCount sell_price = 0;

			while (0 < stack.count && !totalSellPrice(keep_realm, stack, sell_price))
				--stack.count;

			if (stack.count == 0) // Couldn't sell any
				continue;

			auto leftover = keep_realm.stockpileInventory->add(stack);

			if (leftover) {
				stack.count -= leftover->count;
				if (!totalSellPrice(keep_realm, stack, sell_price))
					throw std::runtime_error("Sell price calculation failed after reducing stack");
				new_money += sell_price;
				inventory->remove(stack, slot);
				keep_realm.money -= sell_price;
				break;
			} else {
				new_money += sell_price;
				inventory->remove(stack, slot);
				keep_realm.money -= sell_price;
			}
		}

		setMoney(new_money);
	}

	void to_json(nlohmann::json &json, const Woodcutter &woodcutter) {
		woodcutter.toJSON(json);
	}
}
