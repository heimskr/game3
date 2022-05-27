#include <iostream>

#include "ui/DrawingArea.h"

constexpr static int marching[256] {
	 0,  1,  2,  2,  3,  4,  2,  2,  5,  5,  6,  6,  7,  7,  6,  6,
	 8,  9, 10, 10,  8,  9, 10, 10, 11, 11, 12, 12, 11, 11, 12, 12,
	13, 14, 15, 15, 16, 17, 15, 15,  5,  5,  6,  6,  7,  7,  6,  6,
	18, 19, 20, 20, 18, 19, 20, 20, 11, 11, 12, 12, 11, 11, 12, 12,
	21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27,
	29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33,
	21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27,
	29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33,
	34, 35, 36, 36, 37, 38, 36, 36, 39, 39, 40, 40, 41, 41, 40, 40,
	 8,  9, 10, 10,  8,  9, 10, 10, 11, 11, 12, 12, 11, 11, 12, 12,
	42, 43, 44, 44, 45, 46, 44, 44, 39, 39, 40, 40, 41, 41, 40, 40,
	18, 19, 20, 20, 18, 19, 20, 20, 11, 11, 12, 12, 11, 11, 12, 12,
	21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27,
	29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33,
	21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27,
	29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33
};

static std::unordered_map<int, int> marching_map {
	{  0, 0}, {  2, 1}, {  8, 2}, { 10, 3}, { 11, 4}, { 16, 5}, { 18, 6}, { 22, 7}, { 24, 8}, { 26, 9}, { 27, 10}, { 30, 11}, { 31, 12}, { 64, 13}, { 66, 14}, { 72, 15}, { 74, 16}, { 75, 17}, { 80, 18}, { 82, 19}, { 86, 20}, { 88, 21}, { 90, 22}, { 91, 23}, { 94, 24}, { 95, 25}, {104, 26}, {106, 27}, {107, 28}, {120, 29}, {122, 30}, {123, 31}, {126, 32}, {127, 33}, {208, 34}, {210, 35}, {214, 36}, {216, 37}, {218, 38}, {219, 39}, {222, 40}, {223, 41}, {248, 42}, {250, 43}, {251, 44}, {254, 45}, {255, 46},
};

static std::unordered_map<int, int> modded_marching_map {
	{  0, 40},
	{  2,  1},
	{  8,  2},
	{ 10,  3},
	{ 11, 23},//
	{ 16,  5},
	{ 18,  6},
	{ 22, 21},//
	{ 24,  8},
	{ 26,  9},
	{ 27, 10},
	{ 30, 11},
	{ 31, 22},//
	{ 64, 13},
	{ 66, 14},
	{ 72, 15},
	{ 74, 16},
	{ 75, 17},
	{ 80, 18},
	{ 82, 19},
	{ 86, 20},
	{ 88, 21},
	{ 90, 22},
	{ 91, 23},
	{ 94, 24},
	{ 95, 25},
	{104,  3},//
	{106, 27},
	{107, 13},//
	{120, 29},
	{122, 30},
	{123, 31},
	{126, 32},
	{127,  7},//
	{208,  1},//
	{210, 35},
	{214, 11},//
	{216, 37},
	{218, 38},
	{219, 39},
	{222, 40},
	{223,  9},//
	{248,  2},//
	{250, 43},
	{251, 27},//
	{254, 29},//
	{255, 12},//
};

// constexpr static int ints[][5] = {
// 	{1, 0, 0, 0, 1},
// 	{0, 1, 0, 1, 0},
// 	{0, 0, 1, 0, 1},
// 	{1, 0, 0, 0, 1},
// };

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
		// grass = Cairo::ImageSurface::create_from_png("resources/forest_/forest_.png");
		// grass = Cairo::ImageSurface::create_from_png("resources/forest2.png");
		// grass = Cairo::ImageSurface::create_from_png("resources/48tileset.png");
		grass = Cairo::ImageSurface::create_from_png("resources/grass.png");
		water = Cairo::ImageSurface::create_from_png("resources/lpc/water.png");
		set_draw_func(sigc::mem_fun(*this, &DrawingArea::on_draw));
		// auto click = Gtk::GestureClick::create();
		// click->signal_released().connect([this](int what, double x, double y) {});
		// add_controller(click);
	}

	void DrawingArea::renderTile(const Cairo::RefPtr<Cairo::Context> &cr, Cairo::RefPtr<Cairo::ImageSurface> &tiles,
	                             double canvas_x, double canvas_y, int tile_x_index, int tile_y_index,
	                             double tile_width, double tile_height, const std::string &text) {
		if (tile_height == 0)
			tile_height = tile_width;

		double tile_x = tile_x_index * tile_width;
		double tile_y = tile_y_index * tile_height;
		double scale = 4;

		cr->save();
		cr->scale(scale, scale);
		cr->translate(canvas_x - tile_x, canvas_y - tile_y);
		cr->set_source(tiles, 0, 0);
		cr->translate(tile_x, tile_y);
		cr->rectangle(0., 0., tile_width, tile_height);
		cr->clip();
		cr->paint();
		cr->restore();

		if (!text.empty()) {
			cr->save();
			cr->scale(scale, scale);
			cr->set_font_size(8);
			cr->move_to(canvas_x, canvas_y + 8);
			cr->show_text(text);
			cr->restore();
		}
	}

	void DrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int, int) {
		// renderTile(cr, x_, y_, x_, y_, 16, 16);
		constexpr static int w = sizeof(ints[0]) / sizeof(ints[0][0]);
		constexpr static int h = sizeof(ints) / sizeof(ints[0]);

		static int frame = 0;

		static int r = 0;
		static int c = 0;

		auto get = [&](int x, int y) -> int {
			x += c;
			y += r;
			if (x < 0 || w <= x || y < 0 || h <= y)
				return 0;
			return ints[y][x];
		};

		constexpr int padding = 0;

		for (r = padding; r < h - padding; ++r) {
			for (c = padding; c < w - padding; ++c) {
				int topleft = get(-1, -1), top = get(0, -1), topright = get(1, -1), left = get(-1, 0),
					right = get(1, 0), bottomleft = get(-1, 1), bottom = get(0, 1), bottomright = get(1, 1);
				if (!top || !left) topleft = 0;
				if (!top || !right) topright = 0;
				if (!bottom || !left) bottomleft = 0;
				if (!bottom || !right) bottomright = 0;
				// const int sum = get(-1, -1) + (get(0, -1) << 1) + (get(1, -1) << 2) + (get(-1, 0) << 3) +
				// 	(get(1, 0) << 4) + (get(-1, 1) << 5) + (get(0, 1) << 6) + (get(1, 1) << 7);
				int sum = topleft + (top << 1) + (topright << 2) + (left << 3) + (right << 4) + (bottomleft << 5)
					+ (bottom << 6) + (bottomright << 7);

				if (r == 0 && c == 0) {
					for (int i = -1; i <= 1; ++i) {
						for (int j = -1; j <= 1; ++j)
							std::cerr << get(i, j);
						std::cerr << '\n';
					}
				}
				if (get(0, 0) == 0)
					sum = 0;
				int index = 48;
				// const int sum = get(-1,-1) + 2*get(0,-1) + 4*get(1,-1) + 8*get(-1,0) + 16*get(1,0) + 32*get(-1,1) + 64*get(0,1) + 128*get(1,1);
				// const int index = marching[sum];
				// int index = 48;
				// if (sum == 255)
				// 	index += rand() % 3;
				// else
				// 	index = marching_map.at(sum);
				index = modded_marching_map.at(sum);
				if (index == 12) {
					constexpr static int full[] {12, 30, 41, 41, 41, 41, 41, 41, 41, 41};
					srand((r << 20) | c);
					index = full[rand() % (sizeof(full) / sizeof(full[0]))];
				}
				// int y = 1 + index / 13;
				// int x = 1 + index % 13;
				int y = index / 10;
				int x = index % 10;
				std::cerr << '(' << r << ", " << c << ") -> " << sum << " -> " << index << " -> (" << x << ", " << y << ")\n";
				constexpr int scale = 32;
				renderTile(cr, water, scale * (c - padding), scale * (r - padding), 1, 5, scale, scale);
				renderTile(cr, grass, scale * (c - padding), scale * (r - padding), x, y, scale, scale, std::to_string(sum) + "," + std::to_string(index) + (get(0, 0) == 1? "!" : ""));
			}
		}

		// if (++c == 5) {
		// 	c = 0;
		// 	if (++r == 5)
		// 		r = 0;
		// }



		frame = (frame + 1) % 4;
	}
}
