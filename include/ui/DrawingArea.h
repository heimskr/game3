#pragma once

#include <gtkmm.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

namespace Game3 {
	class MainWindow;

	class DrawingArea: public Gtk::DrawingArea {
		public:
			DrawingArea(MainWindow &);

			void on_draw(const Cairo::RefPtr<Cairo::Context> &, int width, int height);

			double x_ = 1, y_ = 1;

		private:
			MainWindow &mainWindow;
			Cairo::RefPtr<Cairo::ImageSurface> grass;
			Cairo::RefPtr<Cairo::ImageSurface> water;
			void renderTile(const Cairo::RefPtr<Cairo::Context> &, Cairo::RefPtr<Cairo::ImageSurface> &,
			                double canvas_x, double canvas_y, int tile_x_index, int tile_y_index, double tile_width,
			                double tile_height = 0, const std::string &text = "");
	};
}
