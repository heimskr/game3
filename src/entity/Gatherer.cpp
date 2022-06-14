#include <iostream>

#include "Tiles.h"
#include "entity/Gatherer.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/Stonks.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Teleporter.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/tab/InventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	Gatherer::Gatherer(EntityID id_):
		Entity(id_, Entity::GATHERER_TYPE) {}

	Gatherer::Gatherer(EntityID id_, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_):
		Entity(id_, Entity::GATHERER_TYPE), overworldRealm(overworld_realm), houseRealm(house_realm), housePosition(house_position), keep(keep_), keepPosition(keep_->position) {}

	std::shared_ptr<Gatherer> Gatherer::create(EntityID id, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_) {
		auto out = std::shared_ptr<Gatherer>(new Gatherer(id, overworld_realm, house_realm, house_position, keep_));
		out->init();
		return out;
	}

	std::shared_ptr<Gatherer> Gatherer::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Gatherer>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	nlohmann::json Gatherer::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Gatherer::absorbJSON(const nlohmann::json &json) {
		Entity::absorbJSON(json);
		phase          = json.at("phase");
		overworldRealm = json.at("overworldRealm");
		houseRealm     = json.at("house").at(0);
		housePosition  = json.at("house").at(1);
		harvestingTime = json.at("harvestingTime");
		keepPosition   = json.at("keepPosition");
		if (json.contains("chosenResource"))
			chosenResource = json.at("chosenResource");
		if (json.contains("destination"))
			destination = json.at("destination");
	}

	void Gatherer::initAfterRealm() {
		if (!(keep = std::dynamic_pointer_cast<Building>(getRealm()->tileEntityAt(keepPosition))))
			throw std::runtime_error("Couldn't find keep for gatherer");
	}

	void Gatherer::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &tab = *getRealm()->getGame().canvas.window.inventoryTab;
		std::cout << "Gatherer: money = " << money << ", phase = " << int(phase) << '\n';
		player->queueForMove([player, &tab](const auto &) {
			tab.resetExternalInventory();
			return true;
		});
		tab.setExternalInventory("Gatherer", inventory);
	}

	void Gatherer::tick(Game &game, float delta) {
		Entity::tick(game, delta);
		const auto hour = game.getHour();

		if (WORK_START_HOUR <= hour && phase == 0)
			wakeUp();

		if (phase == 1 && realmID == overworldRealm)
			goToResource();

		if (phase == 2 && position == destination)
			startHarvesting();

		if (phase == 3) {
			harvest(delta);
			if (WORK_END_HOUR <= hour)
				phase = 4;
		}

		if (phase == 4)
			goToKeep();

		if (phase == 5 && position == destination)
			goToStockpile();

		if (phase == 6 && position == destination)
			sellInventory();

		if (phase == 7 && SELLING_TIME <= (sellTime += delta))
			leaveKeep();

		if (phase == 8 && getRealm()->id == overworldRealm)
			goToHouse();

		if (phase == 9 && position == destination)
			goToBed();

		if (phase == 10 && position == destination)
			phase = 11;

		if (phase == 11 && hour < WORK_END_HOUR)
			phase = 0;
	}

	void Gatherer::wakeUp() {
		phase = 1;
		auto &game = getRealm()->getGame();
		auto &overworld = *game.realms.at(overworldRealm);
		auto &house     = *game.realms.at(houseRealm);
		const auto width  = overworld.getWidth();
		const auto height = overworld.getHeight();
		const auto &layer2 = *overworld.tilemap2;
		// Detect all resources within a given radius of the house
		std::vector<Index> resource_choices;
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				if (overworldTiles.isResource(layer2(column, row)) && std::sqrt(std::pow(housePosition.row - row, 2) + std::pow(housePosition.column - column, 2)) <= RADIUS)
					resource_choices.push_back(overworld.getIndex(row, column));
		// Choose one at random
		chosenResource = choose(resource_choices, game.dynamicRNG);
		// Pathfind to the door
		pathfind(house.getTileEntity<Teleporter>()->position);
	}

	void Gatherer::goToResource() {
		auto &realm = *getRealm();
		auto chosen_position = realm.getPosition(chosenResource);
		if (auto next = realm.getPathableAdjacent(chosen_position)) {
			phase = 2;
			pathfind(destination = *next);
		} else
			phase = -1;
	}

	void Gatherer::startHarvesting() {
		phase = 3;
		harvestingTime = 0.f;
	}

	void Gatherer::harvest(float delta) {
		if (HARVESTING_TIME <= harvestingTime) {
			harvestingTime = 0.f;
			auto &realm = *getRealm();
			const auto resource_position = realm.getPosition(chosenResource);
			const TileID resource_type = (*getRealm()->tilemap2)(resource_position.column, resource_position.row);
			ItemID item_id = Item::NOTHING;

			switch (resource_type) {
				case OverworldTiles::IRON_ORE:    item_id = Item::IRON_ORE;    break;
				case OverworldTiles::COPPER_ORE:  item_id = Item::COPPER_ORE;  break;
				case OverworldTiles::GOLD_ORE:    item_id = Item::GOLD_ORE;    break;
				case OverworldTiles::DIAMOND_ORE: item_id = Item::DIAMOND_ORE; break;
				case OverworldTiles::COAL_ORE:    item_id = Item::COAL;        break;
				case OverworldTiles::OIL:         item_id = Item::OIL;         break;
				default:
					throw std::runtime_error("Unknown resource type: " + std::to_string(resource_type));
			}

			const ItemStack stack(item_id, 1);
			const auto leftover = inventory->add(stack);
			if (leftover == stack)
				phase = 4;
		} else
			harvestingTime += delta;
	}

	void Gatherer::goToKeep() {
		const auto adjacent = getRealm()->getPathableAdjacent(keep->position);
		if (!adjacent || !pathfind(destination = *adjacent))
			throw std::runtime_error("Gatherer couldn't pathfind to keep");
		phase = 5;
	}

	void Gatherer::goToStockpile() {
		keep->teleport(shared_from_this());
		auto keep_realm = keep->getInnerRealm();
		auto stockpile = keep_realm->getTileEntity<Chest>();
		const auto adjacent = keep_realm->getPathableAdjacent(stockpile->position);
		if (!adjacent || !pathfind(destination = *adjacent))
			throw std::runtime_error("Gatherer couldn't pathfind to stockpile");
		phase = 6;
	}

	void Gatherer::sellInventory() {
		phase = 7;
		auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());
		MoneyCount new_money = money;

		for (Slot slot = 0; slot < inventory->slotCount; ++slot) {
			if (!inventory->contains(slot))
				continue;

			auto stack = inventory->front();
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

	void Gatherer::leaveKeep() {
		phase = 8;
		auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());
		auto door = keep_realm.getTileEntity<Teleporter>([](const auto &door) {
			return door->extraData.contains("exit") && door->extraData.at("exit") == true;
		});
		if (!pathfind(destination = door->position))
			throw std::runtime_error("Gatherer couldn't pathfind to keep door");
	}

	void Gatherer::goToHouse() {
		if (getRealm()->id == overworldRealm) {
			const auto adjacent = getRealm()->getPathableAdjacent(housePosition);
			if (!adjacent || !pathfind(destination = *adjacent))
				throw std::runtime_error("Gatherer couldn't pathfind to house");
			phase = 9;
		}
	}

	void Gatherer::goToBed() {
		auto house = std::dynamic_pointer_cast<Building>(getRealm()->tileEntityAt(housePosition));
		if (!house)
			throw std::runtime_error("Gatherer couldn't find house");
		house->teleport(shared_from_this());

		auto &realm = *getRealm();
		if (realm.id != houseRealm)
			throw std::runtime_error("Gatherer couldn't teleport to house");

		if (!pathfind(destination = realm.extraData.at("bed")))
			throw std::runtime_error("Gatherer couldn't pathfind to bed");

		phase = 10;
	}

	void Gatherer::setMoney(MoneyCount new_money) {
		money = new_money;
	}

	void to_json(nlohmann::json &json, const Gatherer &gatherer) {
		to_json(json, static_cast<const Entity &>(gatherer));
		json["phase"] = gatherer.phase;
		json["overworldRealm"] = gatherer.overworldRealm;
		json["house"][0] = gatherer.houseRealm;
		json["house"][1] = gatherer.housePosition;
		json["harvestingTime"] = gatherer.harvestingTime;
		if (gatherer.chosenResource != -1)
			json["chosenResource"] = gatherer.chosenResource;
		if (gatherer.destination)
			json["destination"] = gatherer.destination;
		json["keepPosition"] = gatherer.keep->position;
	}
}
