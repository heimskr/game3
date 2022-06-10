#include <iostream>

#include "entity/Merchant.h"
#include "game/Game.h"
#include "game/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"

namespace Game3 {
	std::shared_ptr<Merchant> Merchant::create(EntityID id) {
		auto out = std::shared_ptr<Merchant>(new Merchant(id));
		out->init();
		return out;
	}

	std::shared_ptr<Merchant> Merchant::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Merchant>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	nlohmann::json Merchant::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Merchant::absorbJSON(const nlohmann::json &json) {
		Entity::absorbJSON(json);
		priceMultiplier = json.at("priceMultiplier");
	}

	void Merchant::tick(float delta) {
		Entity::tick(delta);
		accumulatedTime += delta;
		if (1.f <= accumulatedTime) {
			accumulatedTime = 0;
			move(lastDirection = remapDirection(lastDirection, 0x3201));
		}
	}

	void to_json(nlohmann::json &json, const Merchant &merchant) {
		to_json(json, static_cast<const Entity &>(merchant));
		json["priceMultiplier"] = merchant.priceMultiplier;
	}
}
