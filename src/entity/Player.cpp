#include <iostream>

#include "entity/Player.h"
#include "game/Realm.h"

namespace Game3 {
	std::shared_ptr<Player> Player::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Player>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	nlohmann::json Player::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Player::interactOn() {
		auto realm = getRealm();
		auto player = std::dynamic_pointer_cast<Player>(shared_from_this());
		auto entity = realm->findEntity(position, player);
		if (!entity)
			return;
		entity->onInteractOn(player);;
	}

	void Player::interactNextTo() {
		auto realm = getRealm();
		auto player = std::dynamic_pointer_cast<Player>(shared_from_this());
		auto entity = realm->findEntity(nextTo(), player);
		if (!entity)
			return;
		entity->onInteractNextTo(player);
	}

	void to_json(nlohmann::json &json, const Player &player) {
		to_json(json, static_cast<const Entity &>(player));
		json["isPlayer"] = true;
	}
}
