#pragma once

#include <memory>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "game/Realm.h"

namespace Game3 {
	class Game {
		public:
			std::unordered_map<int, std::shared_ptr<Realm>> realms;
	};

	void to_json(nlohmann::json &, const Game &);
	void from_json(const nlohmann::json &, Game &);
}
