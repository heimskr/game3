#pragma once

#include <gtkmm.h>

#include "ui/gtk/NumericEntry.h"

namespace Game3 {
	class ConnectDialog: public Gtk::MessageDialog {
		public:
			ConnectDialog(Gtk::Window &);

			inline auto signal_submit() const { return signal_submit_; }

		private:
			sigc::signal<void(const Glib::ustring &, uint16_t)> signal_submit_;
			Gtk::Label hostnameLabel{"Host"};
			Gtk::Label portLabel{"Port"};
			Gtk::Entry hostnameEntry;
			NumericEntry portEntry;

			void submit();
	};
}
