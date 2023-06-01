#pragma once

#include <gtkmm.h>

#include "ui/gtk/NumericEntry.h"

namespace Game3 {
	class MainWindow;

	class ConnectDialog: public Gtk::MessageDialog {
		public:
			ConnectDialog(MainWindow &parent_);

			inline auto signal_submit() const { return signal_submit_; }

		private:
			MainWindow &mainWindow;
			sigc::signal<void(const Glib::ustring &, uint16_t)> signal_submit_;
			Gtk::Label hostnameLabel {"Host"};
			Gtk::Label portLabel {"Port"};
			Gtk::Entry hostnameEntry;
			NumericEntry portEntry;

			void submit();
	};
}
