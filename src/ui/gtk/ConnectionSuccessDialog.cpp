#include "ui/gtk/ConnectionSuccessDialog.h"

namespace Game3 {
	ConnectionSuccessDialog::ConnectionSuccessDialog(Gtk::Window &parent):
	Gtk::MessageDialog(parent, "Connection Succeeded", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::CLOSE, true) {
		auto *area = get_message_area();
		area->append(label);
		area->append(checkButton);
		// int width, height;
		// get_default_size(width, height);
		// if (width < 300)
		// 	set_default_size(300, height);
		checkButton.set_active(false);
		signal_response().connect([this](int) {
			submit();
		});
		set_deletable(false);
	}

	void ConnectionSuccessDialog::submit() {
		hide();
		signal_submit_.emit(checkButton.get_active());
	}
}
