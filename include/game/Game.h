#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>

#include <gtkmm.h>
#include <nlohmann/json.hpp>

#include "entity/Player.h"
#include "game/Realm.h"

namespace Game3 {
	class Canvas;
	class Menu;
	class Player;

	class Game: public std::enable_shared_from_this<Game> {
		public:
			static constexpr const char *DEFAULT_PATH = "game.g3";

			Canvas &canvas;
			/** Seconds since the last tick */
			float delta = 0.f;
			std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
			bool debugMode = true;
			/** 12 because the game starts at noon */
			float hourOffset = 12.;

			Game() = delete;

			std::unordered_map<RealmID, std::shared_ptr<Realm>> realms;
			std::shared_ptr<Realm> activeRealm;
			std::shared_ptr<Player> player;

			void initEntities();
			void tick();
			RealmID newRealmID() const;
			void setText(const Glib::ustring &text, const Glib::ustring &name = "", bool focus = true, bool ephemeral = false);
			const Glib::ustring & getText() const;
			void click(int button, int n, double pos_x, double pos_y);
			float getTotalSeconds() const;
			float getHour() const;
			float getMinute() const;
			/** The value to divide the color values of the tilemap pixels by. Based on the time of day. */
			float getDivisor() const;

			sigc::signal<void(const std::shared_ptr<Player> &)> signal_player_inventory_update() const { return signal_player_inventory_update_; }
			sigc::signal<void(const std::shared_ptr<HasRealm> &)> signal_other_inventory_update()  const { return signal_other_inventory_update_; }

			static std::shared_ptr<Game> create(Canvas &);
			static std::shared_ptr<Game> fromJSON(const nlohmann::json &, Canvas &);

		private:
			Game(Canvas &canvas_): canvas(canvas_) {}
			sigc::signal<void(const std::shared_ptr<Player> &)> signal_player_inventory_update_;
			sigc::signal<void(const std::shared_ptr<HasRealm> &)> signal_other_inventory_update_;
			std::chrono::system_clock::time_point lastTime = startTime;
	};

	void to_json(nlohmann::json &, const Game &);
}
