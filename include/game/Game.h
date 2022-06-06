#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include "game/Player.h"
#include "game/Realm.h"

namespace Game3 {
	class Menu;

	class Game {
		public:
			/** Seconds since the last tick */
			float delta = 0.f;

			std::unordered_map<RealmID, std::shared_ptr<Realm>> realms;
			std::shared_ptr<Realm> activeRealm;
			std::shared_ptr<Player> player;
			// std::shared_ptr<Menu> menu;

			void initEntities();
			void tick();

		private:
			std::chrono::system_clock::time_point lastTime = std::chrono::system_clock::now();
	};

	void to_json(nlohmann::json &, const Game &);
	void from_json(const nlohmann::json &, Game &);
}
