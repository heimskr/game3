#pragma once

#include <gtkmm.h>
#include <memory>

namespace Game3 {
	class ClientGame;

	class Tab {
		public:
			Gtk::Notebook &notebook;

			Tab(Gtk::Notebook &notebook_): notebook(notebook_) {}

			virtual ~Tab() = default;

			virtual Gtk::Widget & getWidget() = 0;
			virtual std::string getName() = 0;
			virtual void onFocus() {}
			virtual void onBlur() {}
			virtual void onResize(const std::shared_ptr<ClientGame> &) {}
			virtual void update(const std::shared_ptr<ClientGame> &) {}
			virtual void reset(const std::shared_ptr<ClientGame> &) {}
			void hide();
			void show();
			void setName(const Glib::ustring &);
			void add();
	};
}
