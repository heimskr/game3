#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <memory>
#include <random>
#include <unordered_map>
#include <utility>

#include <gtkmm.h>
#include <nlohmann/json.hpp>

#include "Types.h"
#include "entity/Player.h"
#include "realm/Realm.h"
#include "registry/Registries.h"
#include "registry/Registry.h"

namespace Game3 {
	class Canvas;
	class MainWindow;
	class Menu;
	class Player;
	struct GhostDetails;
	struct InteractionSet;

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
			/** Seeded with the time. Must not be used to determine deterministic outcomes.
			 *  For example, a Gatherer can use this to choose a resource node to harvest from, but worldgen shouldn't use this. */
			std::default_random_engine dynamicRNG;
			size_t cavesGenerated = 0;
			std::map<RealmType, std::shared_ptr<InteractionSet>> interactionSets;

			Game() = delete;

			std::unordered_map<RealmID, RealmPtr> realms;
			RealmPtr activeRealm;
			PlayerPtr player;

			RegistryRegistry registries;

			template <typename T>
			T & registry() {
				return registries.get<T>();
			}

			template <typename T>
			const T & registry() const {
				return registries.get<const T>();
			}

			void initRegistries();
			void addItems();
			void addGhosts();
			void addEntityFactories();
			void addTileEntityFactories();
			void addRealms();
			void initialSetup(const std::filesystem::path &dir = "data");
			void initEntities();
			void initInteractionSets();
			void add(std::shared_ptr<Item>);
			void add(std::shared_ptr<GhostDetails>);
			void add(EntityFactory &&);
			void add(TileEntityFactory &&);
			void add(RealmFactory &&);
			void traverseData(const std::filesystem::path &);
			void loadDataFile(const std::filesystem::path &);
			void addRecipe(const nlohmann::json &);
			// Returns whether the command executed successfully and a message.
			std::tuple<bool, Glib::ustring> runCommand(const Glib::ustring &);
			void tick();
			RealmID newRealmID() const;
			void setText(const Glib::ustring &text, const Glib::ustring &name = "", bool focus = true, bool ephemeral = false);
			const Glib::ustring & getText() const;
			void click(int button, int n, double pos_x, double pos_y);
			double getTotalSeconds() const;
			double getHour() const;
			double getMinute() const;
			/** The value to divide the color values of the tilemap pixels by. Based on the time of day. */
			double getDivisor() const;
			void activateContext();
			MainWindow & getWindow();
			/** Translates coordinates relative to the top left corner of the canvas to realm coordinates. */
			Position translateCanvasCoordinates(double x, double y) const;
			Gdk::Rectangle getVisibleRealmBounds() const;

			sigc::signal<void(const PlayerPtr &)> signal_player_inventory_update() const { return signal_player_inventory_update_; }
			sigc::signal<void(const PlayerPtr &)> signal_player_money_update() const { return signal_player_money_update_; }
			sigc::signal<void(const std::shared_ptr<HasRealm> &)> signal_other_inventory_update()  const { return signal_other_inventory_update_; }

			static std::shared_ptr<Game> create(Canvas &);
			static std::shared_ptr<Game> fromJSON(const nlohmann::json &, Canvas &);

		private:
			Game(Canvas &canvas_): canvas(canvas_) { dynamicRNG.seed(time(nullptr)); }
			sigc::signal<void(const PlayerPtr &)> signal_player_inventory_update_;
			sigc::signal<void(const PlayerPtr &)> signal_player_money_update_;
			sigc::signal<void(const std::shared_ptr<HasRealm> &)> signal_other_inventory_update_;
			std::chrono::system_clock::time_point lastTime = startTime;
	};

	void to_json(nlohmann::json &, const Game &);

	using GamePtr = std::shared_ptr<Game>;
}
