#include <iostream>
#include <thread>

#include "ui/SFMLArea.h"
#include "ui/MainWindow.h"

#include <gtk-4.0/gdk/x11/gdkx.h>

namespace Game3 {
	SFMLArea::SFMLArea(MainWindow &main_window): mainWindow(main_window) {}

	void SFMLArea::init() {
		std::cerr << "\e[33mwow.\e[39m\n";
		set_can_focus();
		XSetErrorHandler(+[](Display *display, XErrorEvent *error) -> int {
			char errorstring[256];
			XGetErrorText(display, error->error_code, errorstring, 256);
			std::cerr << "X error: " << errorstring << std::endl;
			std::terminate();
			return 0;
		});

		auto surface = mainWindow.get_surface();
		sf::ContextSettings settings;
		settings.depthBits = 24;
		// settings.majorVersion = 3;
		// settings.minorVersion = 2;
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		auto ctx = gdk_gl_context_get_current();
		gdk_gl_context_clear_current();
		auto handle = gdk_x11_surface_get_xid(surface->gobj());
		// renderWindow = std::make_unique<sf::RenderWindow>(handle, settings);
		renderWindow = std::make_unique<sf::RenderWindow>();
		// renderWindow->create(handle, settings);
		gdk_gl_context_make_current(ctx);
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		renderWindow->setFramerateLimit(144);
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		auto alloc = get_allocation();
		sf::View view;
		view.setViewport({
			float(alloc.get_x()), float(alloc.get_y()), float(alloc.get_width()), float(alloc.get_height())
		});
		std::cerr << "thread ID @ " << __LINE__ << ": " << std::this_thread::get_id() << "\n";
		renderWindow->setView(view);
		// renderWindow.create(GDK_SURFACE_XID(surface->gobj()));
	}

	bool SFMLArea::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
		Gtk::GLArea::on_render(context);
		context->clear_current();
		if (renderWindow) {
			std::cerr << "thread ID @ " << __LINE__ << ": " << std::this_thread::get_id() << "\n";
			renderWindow->setActive();
			renderWindow->create(gdk_x11_surface_get_xid(mainWindow.get_surface()->gobj()));
			// context->make_current();
			renderWindow->clear({10, 100, 10, 255});
			renderWindow->display();
		}
		return true;
	}
}
