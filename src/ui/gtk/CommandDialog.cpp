#include "ui/gtk/CommandDialog.h"

namespace Game3 {
	CommandDialog::CommandDialog(Gtk::Window &parent, const Glib::ustring &initial_text):
	Gtk::MessageDialog(parent, "Command", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::OK_CANCEL, true) {
		commandEntry.set_text(initial_text);
		auto *area = get_message_area();
		area->append(commandEntry);
		int width, height;
		get_default_size(width, height);
		if (width < 300)
			set_default_size(300, height);
		commandEntry.signal_activate().connect(sigc::mem_fun(*this, &CommandDialog::submit));
		signal_response().connect([this](int response) {
			if (response == Gtk::ResponseType::OK)
				submit();
			else
				hide();
		});
		set_deletable(false);
	}

	void CommandDialog::submit() {
		hide();
		signal_submit_.emit(commandEntry.get_text());
	}
}
