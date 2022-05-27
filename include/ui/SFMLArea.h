#pragma once

#include <memory>

#include <SFML/Graphics.hpp>
#include <gtkmm.h>

namespace Game3 {
	class MainWindow;

	class SFMLArea: public Gtk::GLArea {
		public:
			SFMLArea(MainWindow &main_window);

			void init();

		protected:
			bool on_render(const Glib::RefPtr<Gdk::GLContext> &) override;

		private:
			MainWindow &mainWindow;
			std::unique_ptr<sf::RenderWindow> renderWindow;
	};
}
