#pragma once

#include <gtkmm.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

namespace Game3 {
	class MainWindow;

	class DrawingArea: public Gtk::GLArea {
		public:
			DrawingArea(MainWindow &);

			~DrawingArea();

		private:
			MainWindow &mainWindow;
			sigc::connection renderConnection;

			SDL_Window *sdlWindow = nullptr;
			SDL_Renderer *sdlRenderer = nullptr;
			SDL_GLContext glContext = nullptr;

			

			void onResize();
			void render();

			bool on_render(const Glib::RefPtr<Gdk::GLContext> &) override;
	};
}
