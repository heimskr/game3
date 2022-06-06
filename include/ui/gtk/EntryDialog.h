#pragma once

#include <gtkmm.h>

namespace Game2 {
	template <typename E>
	class EntryDialog: public Gtk::Dialog {
		public:
			Gtk::Label label;
			E entry;
			Gtk::Box buttons;
			Gtk::Button okay, cancel;

			sigc::signal<void(const Glib::ustring &)> signal_submit() const { return signal_submit_; }

			EntryDialog(const Glib::ustring &title, Gtk::Window &parent, const Glib::ustring &text, bool modal = true):
			Dialog(title, parent, modal) {
				int width, height;
				get_default_size(width, height);
				set_default_size(300, height);
				label.set_text(text);
				label.set_halign(Gtk::Align::START);
				okay.set_label("_OK");
				cancel.set_label("_Cancel");
				buttons.append(cancel);
				buttons.append(okay);
				buttons.set_spacing(5);
				auto &area = *get_content_area();
				area.set_orientation(Gtk::Orientation::VERTICAL);
				area.set_spacing(5);
				area.set_margin_top(5);
				area.set_margin_start(5);
				area.set_margin_end(5);
				area.set_margin_bottom(5);
				buttons.set_halign(Gtk::Align::END);
				entry.set_activates_default(true);
				entry.set_input_purpose(Gtk::InputPurpose::NUMBER);
				entry.signal_activate().connect(sigc::mem_fun(*this, &EntryDialog::submit));
				okay.signal_clicked().connect(sigc::mem_fun(*this, &EntryDialog::submit));
				okay.set_receives_default(true);
				cancel.signal_clicked().connect(sigc::mem_fun(*this, &EntryDialog::hide));
				area.append(label);
				area.append(entry);
				area.append(buttons);
			}

			EntryDialog & set_text(const Glib::ustring &str) { entry.set_text(str); return *this; }

		protected:
			sigc::signal<void(const Glib::ustring &)> signal_submit_;

			void submit() {
				hide();
				signal_submit_.emit(entry.get_text());
			}
	};
}
