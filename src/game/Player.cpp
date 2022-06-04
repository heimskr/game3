#include "game/Player.h"

namespace Game3 {
	nlohmann::json Player::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void to_json(nlohmann::json &json, const Player &player) {
		to_json(json, static_cast<const Entity &>(player));
		json["isPlayer"] = true;
	}

	void from_json(const nlohmann::json &json, Player &player) {
		from_json(json, static_cast<Entity &>(player));
	}
}
