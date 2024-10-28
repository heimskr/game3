#include <iostream>

#include "algorithm/Stonks.h"
#include "entity/Miner.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Teleporter.h"
#include "ui/Window.h"
#include "util/Util.h"

namespace Game3 {
	Miner::Miner():
		Entity(ID()), Worker(ID()) {}

	Miner::Miner(RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_):
		Entity(ID()),
		Worker(ID(), overworld_realm, house_realm, house_position, keep_) {}

	std::shared_ptr<Miner> Miner::create(const std::shared_ptr<Game> &) {
		return Entity::create<Miner>();
	}

	std::shared_ptr<Miner> Miner::create(const std::shared_ptr<Game> &, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_) {
		return Entity::create<Miner>(overworld_realm, house_realm, house_position, keep_);
	}

	std::shared_ptr<Miner> Miner::fromJSON(const GamePtr &game, const nlohmann::json &json) {
		auto out = Entity::create<Miner>();
		out->absorbJSON(game, json);
		return out;
	}

	void Miner::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		Worker::toJSON(json);
		json["harvestingTime"] = harvestingTime;
		if (chosenResource)
			json["chosenResource"] = *chosenResource;
	}

	void Miner::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);
		Worker::absorbJSON(game, json);
		harvestingTime = json.at("harvestingTime");
		if (json.contains("chosenResource"))
			chosenResource = json.at("chosenResource");
	}

	bool Miner::onInteractNextTo(const std::shared_ptr<Player> &player, Modifiers, const ItemStackPtr &, Hand) {
		(void) player;
		std::cout << "Miner: money = " << money << ", phase = " << static_cast<int>(phase) << ", stuck = " << stuck << '\n';

		if (getSide() == Side::Client) {
			// auto &window = getRealm()->getGame()->toClient().getWindow();
			// const auto &tab = window.inventoryTab;
			// player->queueForMove([tab](const auto &, bool) {
			// 	tab->removeModule();
			// 	return true;
			// });

			// TODO
			// window.showExternalInventory(std::dynamic_pointer_cast<ClientInventory>(getInventory(0)));
		}

		return true;
	}

	void Miner::tick(const TickArgs &args) {
		Worker::tick(args);

		const GamePtr &game = args.game;
		const auto delta = args.delta;

		if (getSide() == Side::Client || stillStuck(delta))
			return;

		const auto hour = game->getHour();

		if (phase == 0 && WORK_START_HOUR <= hour)
			wakeUp();

		else if (phase == 1 && realmID == overworldRealm)
			goToResource();

		else if (phase == 2 && position == destination)
			startHarvesting();

		else if (phase == 3) {
			harvest(delta);
			if (WORK_END_HOUR <= hour)
				setPhase(4);
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
			setPhase(11);

		else if (phase == 11 && hour < WORK_END_HOUR)
			setPhase(0);
	}



	void Miner::encode(Buffer &buffer) {
		Entity::encode(buffer);
		Worker::encode(buffer);
		buffer << chosenResource;
		buffer << harvestingTime;
		buffer << sellTime;
	}

	void Miner::decode(Buffer &buffer) {
		Entity::decode(buffer);
		Worker::decode(buffer);
		buffer >> chosenResource;
		buffer >> harvestingTime;
		buffer >> sellTime;
	}

	void Miner::wakeUp() {
		setPhase(1);
		GamePtr game = getRealm()->getGame();
		RealmPtr overworld = game->getRealm(overworldRealm);
		RealmPtr house     = game->getRealm(houseRealm);
		// Detect all resources within a given radius of the house
		std::vector<Position> resource_choices;
		for (const auto &[te_position, tile_entity]: overworld->tileEntities)
			if (dynamic_cast<OreDeposit *>(tile_entity.get()))
				resource_choices.push_back(te_position);
		// If there are no resources, get stuck forever. Seed -1998 has no resources.
		if (resource_choices.empty()) {
			setPhase(-1);
			return;
		}
		// Choose one at random
		chosenResource = choose(resource_choices, threadContext.rng);
		// Pathfind to the door
		pathfind(house->getTileEntity<Teleporter>()->position);
	}

	void Miner::goToResource() {
		auto realm = getRealm();
		if (!chosenResource)
			return;

		if (auto next = realm->getPathableAdjacent(*chosenResource)) {
			if (!pathfind(destination = *next)) {
				stuck = true;
				return;
			}
			setPhase(2);
		} else
			stuck = true;
	}

	void Miner::startHarvesting() {
		setPhase(3);
		harvestingTime = 0.f;
	}

	void Miner::harvest(float delta) {
		if (HARVESTING_TIME <= harvestingTime) {
			harvestingTime = 0.f;
			RealmPtr realm = getRealm();
			auto &deposit = dynamic_cast<OreDeposit &>(*realm->tileEntityAt(*chosenResource));
			GamePtr game = realm->getGame();
			const ItemStackPtr &stack = deposit.getOre(*game).stack;
			const auto leftover = getInventory(0)->add(stack);
			if (leftover == stack)
				setPhase(4);
		} else
			harvestingTime += delta;
	}

	void Miner::sellInventory() {
		// setPhase(7);
		// auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());
		// MoneyCount new_money = money;
		// const InventoryPtr inventory = getInventory(0);

		// for (Slot slot = 0; slot < inventory->getSlotCount(); ++slot) {
		// 	if (!inventory->contains(slot))
		// 		continue;

		// 	ItemStack stack = inventory->front();
		// 	MoneyCount sell_price = 0;

		// 	while (0 < stack.count && !totalSellPrice(keep_realm, stack, sell_price))
		// 		--stack.count;

		// 	if (stack.count == 0) // Couldn't sell any
		// 		continue;

		// 	auto leftover = keep_realm.stockpileInventory->add(stack);

		// 	if (leftover) {
		// 		stack.count -= leftover->count;
		// 		if (!totalSellPrice(keep_realm, stack, sell_price))
		// 			throw std::runtime_error("Sell price calculation failed after reducing stack");
		// 		new_money += sell_price;
		// 		inventory->remove(stack, slot);
		// 		keep_realm.money -= sell_price;
		// 		break;
		// 	} else {
		// 		new_money += sell_price;
		// 		inventory->remove(stack, slot);
		// 		keep_realm.money -= sell_price;
		// 	}
		// }

		// setMoney(new_money);
	}

	void to_json(nlohmann::json &json, const Miner &miner) {
		miner.toJSON(json);
	}
}
