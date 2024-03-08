#pragma once

#include <string>

#include <gtkmm.h>
#include <vte/vte.h>

namespace Game3 {
	class LogOverlay: public Gtk::ScrolledWindow {
		public:
			LogOverlay();

			void reset();
			void print(std::string_view);

		private:
			Gtk::Widget *terminal = nullptr;
			VteTerminal *vte = nullptr;
	};
}
