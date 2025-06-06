#include "threading/ThreadContext.h"
#include "entity/Worker.h"
#include "game/Game.h"
#include "lib/JSON.h"
#include "net/Buffer.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Teleporter.h"

namespace Game3 {
	Worker::Worker(EntityType type_):
		Entity(std::move(type_)) {}

	Worker::Worker(EntityType type_, RealmID overworld_realm, RealmID house_realm, Position house_position, std::shared_ptr<Building> keep_):
		Entity(std::move(type_)),
		overworldRealm(overworld_realm),
		houseRealm(house_realm),
		housePosition(house_position),
		keep(std::move(keep_)),
		keepPosition(keep->position) {}

	void Worker::toJSON(boost::json::value &json) const {
		LivingEntity::toJSON(json);
		auto &object = json.as_object();
		object["phase"] = phase;
		object["overworldRealm"] = overworldRealm;
		auto &house_array = object["house"].emplace_array();
		house_array[0] = boost::json::value_from(houseRealm);
		house_array[1] = boost::json::value_from(housePosition);
		object["keepPosition"] = boost::json::value_from(keep->position);
		if (stuck)
			object["stuck"] = stuck;
		object["destination"] = boost::json::value_from(destination);
	}

	void Worker::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		LivingEntity::absorbJSON(game, json);
		const auto &object = json.as_object();
		const auto &house_array = object.at("house").as_array();
		phase          = getNumber<Phase>(object.at("phase"));
		overworldRealm = boost::json::value_to<RealmID>(object.at("overworldRealm"));
		houseRealm     = boost::json::value_to<RealmID>(house_array.at(0));
		housePosition  = boost::json::value_to<Position>(house_array.at(1));
		keepPosition   = boost::json::value_to<Position>(object.at("keepPosition"));
		if (auto iter = object.find("destination"); iter != object.end()) {
			destination = boost::json::value_to<Position>(iter->value());
		}
		if (auto iter = object.find("stuck"); iter != object.end()) {
			stuck = iter->value().as_bool();
		}
	}

	void Worker::initAfterLoad(Game &game) {
		if (!(keep = std::dynamic_pointer_cast<Building>(game.getRealm(overworldRealm)->tileEntityAt(keepPosition))))
			throw std::runtime_error("Couldn't find keep for worker");
	}

	void Worker::encode(Buffer &buffer) {
		LivingEntity::encode(buffer);
		buffer << phase;
		buffer << overworldRealm;
		buffer << houseRealm;
		buffer << keepPosition;
		buffer << destination;
		buffer << stuck;
		buffer << stuckTime;
	}

	void Worker::decode(Buffer &buffer) {
		LivingEntity::decode(buffer);
		buffer >> phase;
		buffer >> overworldRealm;
		buffer >> houseRealm;
		buffer >> keepPosition;
		buffer >> destination;
		buffer >> stuck;
		buffer >> stuckTime;
	}

	bool Worker::stillStuck(float delta) {
		if (stuck) {
			if ((stuckTime += delta) < RETRY_TIME)
				return true;
			stuck = false;
			stuckTime = std::uniform_real_distribution(0.f, RETRY_TIME * .8f)(threadContext.rng);
		}

		return false;
	}

	void Worker::goToKeep(Phase new_phase) {
		const auto adjacent = getRealm()->getPathableAdjacent(keep->position);
		if (!adjacent || !pathfind(destination = *adjacent))
			// throw std::runtime_error("Worker couldn't pathfind to keep");
			stuck = true;
		else
			setPhase(new_phase);
	}

	void Worker::goToStockpile(Phase new_phase) {
		keep->teleport(getSelf());
		auto keep_realm = keep->getInnerRealm();
		auto stockpile = keep_realm->getTileEntity<Chest>();
		const auto adjacent = keep_realm->getPathableAdjacent(stockpile->position);
		if (!adjacent || !pathfind(destination = *adjacent))
			// throw std::runtime_error("Worker couldn't pathfind to stockpile");
			stuck = true;
		else
			phase = new_phase;
		increaseUpdateCounter();
	}

	void Worker::leaveKeep(Phase) {
		// setPhase(new_phase);
		// auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());
		// auto door = keep_realm.getTileEntity<Teleporter>([](const auto &door) {
		// 	return door->extraData.contains("exit") && door->extraData.at("exit") == true;
		// });
		// if (!pathfind(destination = door->position)) {
		// 	// throw std::runtime_error("Worker couldn't pathfind to keep door");
		// 	stuck = true;
		// 	return;
		// }
	}

	void Worker::goToHouse(Phase new_phase) {
		if (getRealm()->id == overworldRealm) {
			const auto adjacent = getRealm()->getPathableAdjacent(housePosition);
			if (!adjacent || !pathfind(destination = *adjacent)) {
				// throw std::runtime_error("Worker couldn't pathfind to house");
				stuck = true;
				increaseUpdateCounter();
				return;
			}
			setPhase(new_phase);
		}
	}

	void Worker::goToBed(Phase new_phase) {
		RealmPtr realm = getRealm();
		GamePtr game = realm->getGame();

		auto house = std::dynamic_pointer_cast<Building>(game->getRealm(overworldRealm)->tileEntityAt(housePosition));
		if (!house)
			throw std::runtime_error("Worker of type " + type.str() + " couldn't find house at " + std::string(housePosition));
		house->teleport(getSelf());

		if (realm->id != houseRealm) {
			// throw std::runtime_error("Worker couldn't teleport to house");
			stuck = true;
			return;
		}

		destination = boost::json::value_to<Position>(realm->extraData.at("bed"));

		if (!pathfind(destination)) {
			// throw std::runtime_error("Worker couldn't pathfind to bed");
			stuck = true;
			return;
		}

		setPhase(new_phase);
	}

	void Worker::setPhase(Phase new_phase) {
		phase = new_phase;
		increaseUpdateCounter();
	}
}
