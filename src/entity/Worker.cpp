#include "entity/Worker.h"
#include "game/Game.h"
#include "realm/Keep.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Teleporter.h"

namespace Game3 {
	Worker::Worker(EntityID id_, EntityType type_):
		Entity(id_, type_) {}

	Worker::Worker(EntityID id_, EntityType type_, RealmID overworld_realm, RealmID house_realm, const Position &house_position, const std::shared_ptr<Building> &keep_):
		Entity(id_, type_), overworldRealm(overworld_realm), houseRealm(house_realm), housePosition(house_position), keep(keep_), keepPosition(keep_->position) {}

	void Worker::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);
		json["phase"] = phase;
		json["overworldRealm"] = overworldRealm;
		json["house"][0] = houseRealm;
		json["house"][1] = housePosition;
		json["keepPosition"] = keep->position;
		if (stuck)
			json["stuck"] = stuck;
		if (destination)
			json["destination"] = destination;
	}

	void Worker::absorbJSON(const nlohmann::json &json) {
		Entity::absorbJSON(json);
		phase          = json.at("phase");
		overworldRealm = json.at("overworldRealm");
		houseRealm     = json.at("house").at(0);
		housePosition  = json.at("house").at(1);
		keepPosition   = json.at("keepPosition");
		if (json.contains("destination"))
			destination = json.at("destination");
		if (json.contains("stuck"))
			stuck = json.at("stuck");
	}

	void Worker::initAfterLoad(Game &game) {
		if (!(keep = std::dynamic_pointer_cast<Building>(game.realms.at(overworldRealm)->tileEntityAt(keepPosition))))
			throw std::runtime_error("Couldn't find keep for worker");
	}
	
	bool Worker::stillStuck(float delta) {
		if (stuck) {
			if ((stuckTime += delta) < RETRY_TIME)
				return true;
			stuck = false;
			stuckTime = (rand() % static_cast<int>(RETRY_TIME * 100)) / 100.f;
		}

		return false;
	}

	void Worker::goToKeep(Phase new_phase) {
		const auto adjacent = getRealm()->getPathableAdjacent(keep->position);
		if (!adjacent || !pathfind(destination = *adjacent))
			// throw std::runtime_error("Worker couldn't pathfind to keep");
			stuck = true;
		else
			phase = new_phase;
	}

	void Worker::goToStockpile(Phase new_phase) {
		keep->teleport(shared_from_this());
		auto keep_realm = keep->getInnerRealm();
		auto stockpile = keep_realm->getTileEntity<Chest>();
		const auto adjacent = keep_realm->getPathableAdjacent(stockpile->position);
		if (!adjacent || !pathfind(destination = *adjacent))
			// throw std::runtime_error("Worker couldn't pathfind to stockpile");
			stuck = true;
		else
			phase = new_phase;
	}

	void Worker::leaveKeep(Phase new_phase) {
		phase = new_phase;
		auto &keep_realm = dynamic_cast<Keep &>(*keep->getInnerRealm());
		auto door = keep_realm.getTileEntity<Teleporter>([](const auto &door) {
			return door->extraData.contains("exit") && door->extraData.at("exit") == true;
		});
		if (!pathfind(destination = door->position)) {
			// throw std::runtime_error("Worker couldn't pathfind to keep door");
			stuck = true;
			return;
		}
	}

	void Worker::goToHouse(Phase new_phase) {
		if (getRealm()->id == overworldRealm) {
			const auto adjacent = getRealm()->getPathableAdjacent(housePosition);
			if (!adjacent || !pathfind(destination = *adjacent)) {
				// throw std::runtime_error("Worker couldn't pathfind to house");
				stuck = true;
				return;
			}
			phase = new_phase;
		}
	}

	void Worker::goToBed(Phase new_phase) {
		auto house = std::dynamic_pointer_cast<Building>(getRealm()->getGame().realms.at(overworldRealm)->tileEntityAt(housePosition));
		if (!house)
			throw std::runtime_error("Worker of type " + std::to_string(static_cast<int>(type)) + " couldn't find house at " + std::string(housePosition));
		house->teleport(shared_from_this());

		auto &realm = *getRealm();
		if (realm.id != houseRealm) {
			// throw std::runtime_error("Worker couldn't teleport to house");
			stuck = true;
			return;
		}

		if (!pathfind(destination = realm.extraData.at("bed"))) {
			// throw std::runtime_error("Worker couldn't pathfind to bed");
			stuck = true;
			return;
		}

		phase = new_phase;
	}
}
