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
		interact(position);
	}

	void Player::interactNextTo() {
		interact(nextTo());
	}

	void Player::interact(const Position &where) {
		auto realm = getRealm();
		auto entity = realm->findEntity(where, shared_from_this());
		if (!entity)
			return;
		std::cout << "Interacting with entity " << entity->debug() << "\n";
	}

	void to_json(nlohmann::json &json, const Player &player) {
		to_json(json, static_cast<const Entity &>(player));
		json["isPlayer"] = true;
	}
}
