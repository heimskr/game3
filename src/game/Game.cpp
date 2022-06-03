#include "game/Game.h"

namespace Game3 {

	void to_json(nlohmann::json &json, const Game &game) {
		json["realms"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[id, realm]: game.realms)
			json["realms"][std::to_string(id)] = nlohmann::json(*realm);
	}
}
