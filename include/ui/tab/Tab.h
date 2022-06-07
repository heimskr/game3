#pragma once

#include <gtkmm.h>
#include <memory>

namespace Game3 {
	class Game;

	class Tab {
		public:
			virtual Gtk::Widget & getWidget() = 0;
			virtual Glib::ustring getName() = 0;
			virtual void onFocus() {}
			virtual void onBlur() {}
			virtual void onResize(const std::shared_ptr<Game> &) {}
			virtual void update(const std::shared_ptr<Game> &) {}
			virtual void reset(const std::shared_ptr<Game> &) {}
			virtual ~Tab() {}
	};
}
