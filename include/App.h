#pragma once

#include <gtkmm.h>
#include <string>

namespace Game3 {
	class MainWindow;

	class App: public Gtk::Application {
		public:
			static Glib::RefPtr<App> create();

			void on_startup() override;
			void on_activate() override;

			MainWindow * create_window();

			static const char * get_text(const std::string &path, gsize &);
			static const char * get_text(const std::string &path);

		protected:
			App();

		private:
			void on_hide_window(Gtk::Window *);
	};
}
