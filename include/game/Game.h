#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>

#include <gtkmm.h>
#include <nlohmann/json.hpp>

#include "entity/Player.h"
#include "game/Realm.h"

namespace Game3 {
	class Menu;
	class Player;

	class Game {
		public:
			static constexpr const char *DEFAULT_PATH = "game.g3";
			/** Seconds since the last tick */
			float delta = 0.f;

			std::unordered_map<RealmID, std::shared_ptr<Realm>> realms;
			std::shared_ptr<Realm> activeRealm;
			std::shared_ptr<Player> player;

			void initEntities();
			void tick();

			sigc::signal<void(const std::shared_ptr<Player> &)> signal_player_inventory_update() const { return signal_player_inventory_update_; }

		private:
			sigc::signal<void(const std::shared_ptr<Player> &)> signal_player_inventory_update_;
			std::chrono::system_clock::time_point lastTime = std::chrono::system_clock::now();
	};

	void to_json(nlohmann::json &, const Game &);
	void from_json(const nlohmann::json &, Game &);
}
