#include <iostream>

#include "MarchingSquares.h"
#include "ui/DrawingArea.h"
#include "ui/MainWindow.h"

#include <gtk-4.0/gdk/x11/gdkx.h>

constexpr static int ints[][9] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 0, 0, 0, 1, 1, 1},
	{1, 1, 1, 0, 0, 0, 1, 1, 1},
	{1, 1, 1, 0, 0, 0, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1},
};

namespace Game3 {
	DrawingArea::DrawingArea(MainWindow &main_window): mainWindow(main_window) {
		// grass = Cairo::ImageSurface::create_from_png("resources/grass.png");
		// water = Cairo::ImageSurface::create_from_png("resources/lpc/water.png");
		// set_draw_func(sigc::mem_fun(*this, &DrawingArea::on_draw));
		// auto click = Gtk::GestureClick::create();
		// click->signal_released().connect([this](int what, double x, double y) {});
		// add_controller(click);

		mainWindow.delay([this] {
			auto handle = gdk_x11_surface_get_xid(mainWindow.get_surface()->gobj());
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			SDL_Init(SDL_INIT_VIDEO);
			// sdlWindow = SDL_CreateWindowFrom((void *) handle);
			auto alloc = get_allocation();
			sdlWindow = SDL_CreateWindow("Yikes", alloc.get_x(), alloc.get_y(), alloc.get_width(), alloc.get_height(), SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);
			// SDL_HideWindow(sdlWindow);
			glContext = SDL_GL_CreateContext(sdlWindow);
			sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
			if (!sdlRenderer) {
				std::cerr << "\e[2m[\e[22;31m!\e[39;2m]\e[22m " << SDL_GetError() << '\n';
				return;
			}


			onResize();

			renderConnection = Glib::signal_idle().connect([this]() {
				queue_render();
				return true;
			});
		}, 2);

		signal_resize().connect([this](int, int) {
			onResize();
		});
	}

	DrawingArea::~DrawingArea() {
		renderConnection.disconnect();
		if (sdlWindow)
			SDL_DestroyWindow(sdlWindow);
	}

	void DrawingArea::onResize() {
		// return;

		if (sdlRenderer) {
			const auto alloc = get_allocation();
			int padding = 20;
			// SDL_Rect rect {alloc.get_x() + padding, alloc.get_y() + padding, alloc.get_width() - 2 * padding, alloc.get_height() - 2 * padding};
			SDL_Rect rect;
			rect.x = 10;
			rect.y = 10;
			rect.w = 20;
			rect.h = 10;
			std::cerr << alloc.get_width() << ", " << alloc.get_height() << '\n';
			// SDL_RenderSetClipRect(sdlRenderer, &rect);
			if (SDL_RenderSetViewport(sdlRenderer, &rect) != 0) {
				std::cerr << "!! " << SDL_GetError() << '\n';
			} else std::cerr << ":)\n";

			// SDL_SetWindowSize(sdlWindow, alloc.get_width(), alloc.get_height());

			// SDL_SetWindowBordered(sdlWindow, SDL_TRU);
		}
	}

	void DrawingArea::render() {
		if (!sdlRenderer)
			return;

		SDL_SetRenderDrawColor(sdlRenderer, 50, 0, 0, 1);
		// SDL_RenderClear(sdlRenderer);
		SDL_RenderPresent(sdlRenderer);
	}

	bool DrawingArea::on_render(const Glib::RefPtr<Gdk::GLContext> &context) {
		Gtk::GLArea::on_render(context);

		if (sdlRenderer) {
			std::cerr << ".\n";
			// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// SDL_GL_SwapBuffers();

			glClearColor (0.5, 0.5, 0.5, 1.0);
			glClear (GL_COLOR_BUFFER_BIT);
			
			// SDL_GL_MakeCurrent(sdlWindow, glContext);
			context->make_current();
			SDL_SetRenderDrawColor(sdlRenderer, 50, 0, 0, 1);
			SDL_RenderClear(sdlRenderer);
			SDL_RenderPresent(sdlRenderer);
			SDL_GL_SwapWindow(sdlWindow);
		}

		queue_render();


		return true;
	}

	// void DrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int, int) {
	// 	constexpr static int w = sizeof(ints[0]) / sizeof(ints[0][0]);
	// 	constexpr static int h = sizeof(ints) / sizeof(ints[0]);

	// 	static int frame = 0;

	// 	static int r = 0;
	// 	static int c = 0;

	// 	auto get = [&](int x, int y) -> int {
	// 		x += c;
	// 		y += r;
	// 		if (x < 0 || w <= x || y < 0 || h <= y)
	// 			return 0;
	// 		return ints[y][x];
	// 	};

	// 	constexpr int padding = 0;

	// 	for (r = padding; r < h - padding; ++r) {
	// 		for (c = padding; c < w - padding; ++c) {
	// 			int topleft = get(-1, -1), top = get(0, -1), topright = get(1, -1), left = get(-1, 0),
	// 				right = get(1, 0), bottomleft = get(-1, 1), bottom = get(0, 1), bottomright = get(1, 1);
	// 			if (!top || !left) topleft = 0;
	// 			if (!top || !right) topright = 0;
	// 			if (!bottom || !left) bottomleft = 0;
	// 			if (!bottom || !right) bottomright = 0;
	// 			const int sum = get(0, 0) * (topleft + (top << 1) + (topright << 2) + (left << 3) + (right << 4) +
	// 			                (bottomleft << 5) + (bottom << 6) + (bottomright << 7));
	// 			int index = modded_marching_map.at(sum);
	// 			if (index == 12) {
	// 				constexpr static int full[] {12, 30, 41, 41, 41, 41, 41, 41, 41, 41};
	// 				srand((r << 20) | c);
	// 				index = full[rand() % (sizeof(full) / sizeof(full[0]))];
	// 			}
	// 			int y = index / 10;
	// 			int x = index % 10;
	// 			constexpr int scale = 32;
	// 			renderTile(cr, water, scale * (c - padding), scale * (r - padding), 1, 5, scale, scale);
	// 			renderTile(cr, grass, scale * (c - padding), scale * (r - padding), x, y, scale, scale, std::to_string(sum) + "," + std::to_string(index) + (get(0, 0) == 1? "!" : ""));
	// 		}
	// 	}
	
	// 	frame = (frame + 1) % 4;
	// }
}
