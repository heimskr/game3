#pragma once

#include <gtkmm.h>

namespace Game3 {
	class EntryDialog: public Gtk::MessageDialog {
		public:
			EntryDialog(Gtk::Window &parent, const Glib::ustring &message, const Glib::ustring &initial_text = {});

			inline sigc::signal<void(const Glib::ustring &)> signal_submit() const { return signal_submit_; }

		private:
			sigc::signal<void(const Glib::ustring &)> signal_submit_;
			Gtk::Entry entry;

			void submit();
	};
}
