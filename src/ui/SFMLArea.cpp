#include <iostream>
#include <thread>

#include "ui/SFMLArea.h"
#include "ui/MainWindow.h"

#include <gtk-4.0/gdk/x11/gdkx.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>


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

		glEnable(GL_DEBUG_OUTPUT);
		// glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		// glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageCallback(+[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
			if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
				return;
			std::cerr << "\e[2m[\e[22;31m!\e[39;2m]\e[22m " << message << '\n';
		}, nullptr);

		auto surface = mainWindow.get_surface();
		sf::ContextSettings settings;
		// settings.depthBits = 24;
		settings.majorVersion = 3;
		settings.minorVersion = 2;
		settings.attributeFlags |= sf::ContextSettings::Attribute::Core;
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		glContext = gdk_gl_context_get_current();
		// gdk_gl_context_clear_current();
		auto handle = gdk_x11_surface_get_xid(surface->gobj());
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		renderWindow = std::make_unique<sf::RenderWindow>(handle);
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		// renderWindow = std::make_unique<sf::RenderWindow>();
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		// renderWindow->create(handle, settings);
		std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		// gdk_gl_context_make_current(ctx);
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
		std::cerr << "\e[34mon_render\e[39m\n";
		// Gtk::GLArea::on_render(context);
		// context->clear_current();
		if (renderWindow) {
			gdk_gl_context_clear_current();
			// gdk_gl_context_make_current(glContext);
			std::cerr << "thread ID @ " << __LINE__ << ": " << std::this_thread::get_id() << "\n";
			renderWindow->setActive();
			// renderWindow->create(gdk_x11_surface_get_xid(mainWindow.get_surface()->gobj()));
			// context->make_current();
			std::cerr << __FILE__ << ":" << __LINE__ << "\n";
			renderWindow->clear({10, 100, 10, 255});
			std::cerr << __FILE__ << ":" << __LINE__ << "\n";
			renderWindow->display();
			std::cerr << __FILE__ << ":" << __LINE__ << "\n";
		}
		return true;
	}
}
