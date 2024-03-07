#include "ui/gtk/EntryDialog.h"

namespace Game3 {
	EntryDialog::EntryDialog(Gtk::Window &parent, const Glib::ustring &message, const Glib::ustring &initial_text):
	Gtk::MessageDialog(parent, message, false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::OK_CANCEL, true) {
		entry.set_text(initial_text);
		auto *area = get_message_area();
		area->append(entry);
		int width{}, height{};
		get_default_size(width, height);
		if (width < 300)
			set_default_size(300, height);
		entry.signal_activate().connect(sigc::mem_fun(*this, &EntryDialog::submit));
		signal_response().connect([this](int response) {
			if (response == Gtk::ResponseType::OK)
				submit();
			else
				hide();
		});
		set_deletable(false);
	}

	void EntryDialog::submit() {
		hide();
		signal_submit_.emit(entry.get_text());
	}
}
