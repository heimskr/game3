#pragma once

#include <gtkmm.h>

namespace Game3 {
	class LoginDialog: public Gtk::MessageDialog {
		public:
			LoginDialog(Gtk::Window &parent, const Glib::ustring &initial_username = {});

			inline sigc::signal<void(const Glib::ustring &username, const Glib::ustring &display_name)> signal_submit() const { return signal_submit_; }

		private:
			sigc::signal<void(const Glib::ustring &username, const Glib::ustring &display_name)> signal_submit_;
			Gtk::Entry usernameEntry;
			Gtk::Entry displayNameEntry;

			void submit();
	};
}
