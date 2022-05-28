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

			~DrawingArea();

			void on_draw(const Cairo::RefPtr<Cairo::Context> &, int width, int height);

		private:
			MainWindow &mainWindow;
			sigc::connection renderConnection;
	};
}
