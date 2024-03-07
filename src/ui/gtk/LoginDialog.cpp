#include "ui/gtk/LoginDialog.h"

namespace Game3 {
	LoginDialog::LoginDialog(Gtk::Window &parent, const Glib::ustring &initial_username):
	Gtk::MessageDialog(parent, "Log In", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::OK_CANCEL, true) {
		usernameEntry.set_text(initial_username);
		usernameEntry.set_placeholder_text("Username");
		displayNameEntry.set_placeholder_text("Display Name");
		auto *area = get_message_area();
		area->append(usernameEntry);
		area->append(displayNameEntry);
		int width{}, height{};
		get_default_size(width, height);
		if (width < 300)
			set_default_size(300, height);
		usernameEntry.signal_activate().connect(sigc::mem_fun(*this, &LoginDialog::submit));
		displayNameEntry.signal_activate().connect(sigc::mem_fun(*this, &LoginDialog::submit));
		signal_response().connect([this](int response) {
			if (response == Gtk::ResponseType::OK) {
				if (!usernameEntry.get_text().empty() && !displayNameEntry.get_text().empty())
					submit();
			} else {
				hide();
			}
		});
		set_deletable(false);
	}

	void LoginDialog::submit() {
		hide();
		signal_submit_.emit(usernameEntry.get_text(), displayNameEntry.get_text());
	}
}
