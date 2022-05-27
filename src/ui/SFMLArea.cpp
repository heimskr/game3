#include <iostream>
#include <thread>

#include "ui/SFMLArea.h"
#include "ui/MainWindow.h"

#include <gtk-4.0/gdk/x11/gdkx.h>

namespace Game3 {
	SFMLArea::SFMLArea(MainWindow &main_window): mainWindow(main_window) {}

	void SFMLArea::init() {
		set_can_focus();
		auto surface = mainWindow.get_surface();
		sf::ContextSettings settings;
		settings.depthBits = 24;
		settings.majorVersion = 3;
		settings.minorVersion = 2;
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		renderWindow = std::make_unique<sf::RenderWindow>(gdk_x11_surface_get_xid(surface->gobj()), settings);
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		renderWindow->setFramerateLimit(144);
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		auto alloc = get_allocation();
		sf::View view;
		view.setViewport({
			alloc.get_x(), alloc.get_y(), alloc.get_width(), alloc.get_height()
		});
		std::cerr << "thread ID @ " << __LINE__ << ": " << std::this_thread::get_id() << "\n";
		renderWindow->setView(view);
		// renderWindow.create(GDK_SURFACE_XID(surface->gobj()));
	}

	bool SFMLArea::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
		Gtk::GLArea::on_render(context);
		if (renderWindow) {
			std::cerr << "thread ID @ " << __LINE__ << ": " << std::this_thread::get_id() << "\n";
			renderWindow->setActive();
			renderWindow->clear({10, 10, 10, 255});
			renderWindow->display();
		}
		return true;
	}
}
