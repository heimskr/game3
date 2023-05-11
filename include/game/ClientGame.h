#pragma once

#include "game/Game.h"

namespace Game3 {
	class LocalClient;

	class ClientGame: public Game {
		public:
			Canvas &canvas;
			PlayerPtr player;

			std::shared_ptr<LocalClient> client;

			ClientGame(Canvas &canvas_): Game(), canvas(canvas_) {}

			void click(int button, int n, double pos_x, double pos_y);
			Gdk::Rectangle getVisibleRealmBounds() const;
			MainWindow & getWindow();
			/** Translates coordinates relative to the top left corner of the canvas to realm coordinates. */
			Position translateCanvasCoordinates(double x, double y) const;
			void activateContext();
			void setText(const Glib::ustring &text, const Glib::ustring &name = "", bool focus = true, bool ephemeral = false);
			const Glib::ustring & getText() const;
			void runCommand(const std::string &);
			void tick();

			sigc::signal<void(const PlayerPtr &)> signal_player_inventory_update() const { return signal_player_inventory_update_; }
			sigc::signal<void(const PlayerPtr &)> signal_player_money_update() const { return signal_player_money_update_; }
			sigc::signal<void(const std::shared_ptr<HasRealm> &)> signal_other_inventory_update()  const { return signal_other_inventory_update_; }

			Side getSide() const override { return Side::Client; }

		private:
			sigc::signal<void(const PlayerPtr &)> signal_player_inventory_update_;
			sigc::signal<void(const PlayerPtr &)> signal_player_money_update_;
			sigc::signal<void(const std::shared_ptr<HasRealm> &)> signal_other_inventory_update_;
	};
}
