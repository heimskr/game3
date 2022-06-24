#include "entity/Worker.h"
#include "game/Game.h"
#include "tileentity/Building.h"

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
}
