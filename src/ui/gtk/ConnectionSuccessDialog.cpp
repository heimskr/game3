#include "ui/gtk/ConnectionSuccessDialog.h"

namespace Game3 {
	ConnectionSuccessDialog::ConnectionSuccessDialog(Gtk::Window &parent):
	Gtk::MessageDialog(parent, "Connection Succeeded", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::CLOSE, true) {
		auto *area = get_message_area();
		area->append(label);
		area->append(checkButton);
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
