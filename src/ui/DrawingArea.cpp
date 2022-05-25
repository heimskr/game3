#include <iostream>

#include "ui/DrawingArea.h"

namespace Game3 {
	DrawingArea::DrawingArea(MainWindow &main_window): mainWindow(main_window) {
		tiles = Cairo::ImageSurface::create_from_png("resources/forest_/forest_.png");
		surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, tiles->get_width(), tiles->get_height());
		set_draw_func(sigc::mem_fun(*this, &DrawingArea::on_draw));
	}

	void DrawingArea::renderTile(const Cairo::RefPtr<Cairo::Context> &cr, double canvas_x, double canvas_y,
	                             double tile_x, double tile_y, double tile_width, double tile_height) {
		if (tile_height == 0)
			tile_height = tile_width;

		tile_x *= tile_width;
		tile_y *= tile_height;

		cr->translate(canvas_x - tile_x, canvas_y - tile_y);
		cr->set_source(tiles, 0, 0);
		cr->translate(tile_x, tile_y);
		cr->rectangle(0., 0., tile_width, tile_height);
		cr->clip();
		cr->paint();
	}

	void DrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height) {
		renderTile(cr, x_, y_, x_, y_, 16, 16);
	}
}
