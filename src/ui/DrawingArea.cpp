#include <iostream>

#include <SDL2/SDL.h>

#include "ui/DrawingArea.h"
#include "MarchingSquares.h"

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
		set_draw_func(sigc::mem_fun(*this, &DrawingArea::on_draw));
		// auto click = Gtk::GestureClick::create();
		// click->signal_released().connect([this](int what, double x, double y) {});
		// add_controller(click);
		SDL_Init(SDL_INIT_VIDEO);

		renderConnection = Glib::signal_timeout().connect([this]() -> bool {
			return true;
		}, 1000 / 144);
	}

	DrawingArea::~DrawingArea() {
		renderConnection.disconnect();
	}

	void DrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &, int, int) {

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
