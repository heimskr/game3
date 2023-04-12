#pragma once

#include <gtkmm.h>

namespace Game3 {
	class CommandDialog: public Gtk::MessageDialog {
		public:
			CommandDialog(Gtk::Window &parent, const Glib::ustring &initial_text = {});

			inline sigc::signal<void(const Glib::ustring &)> signal_submit() const { return signal_submit_; }

		private:
			sigc::signal<void(const Glib::ustring &)> signal_submit_;
			Gtk::Entry commandEntry;

			void submit();
	};
}
