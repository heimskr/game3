#include "ui/gtk/ConnectDialog.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	ConnectDialog::ConnectDialog(Gtk::Window &parent_): Gtk::MessageDialog(parent_, "Connect", false, Gtk::MessageType::OTHER, Gtk::ButtonsType::CANCEL, true) {
		auto *area = get_message_area();
		hostnameLabel.set_halign(Gtk::Align::START);
		portLabel.set_halign(Gtk::Align::START);
		hostnameEntry.set_text("::1");
		hostnameEntry.signal_activate().connect(sigc::mem_fun(*this, &ConnectDialog::submit));
		portEntry.set_text("12255");
		portEntry.signal_activate().connect(sigc::mem_fun(*this, &ConnectDialog::submit));
		area->append(hostnameLabel);
		area->append(hostnameEntry);
		area->append(portLabel);
		area->append(portEntry);
		add_button("C_onnect", Gtk::ResponseType::OK);
		int width, height;
		get_default_size(width, height);
		if (width < 300)
			set_default_size(300, height);
		signal_response().connect([this](int response) {
			if (response == Gtk::ResponseType::OK)
				submit();
			else
				hide();
		});
		set_deletable(false);
	}

	void ConnectDialog::submit() {
		hide();

		uint16_t port = 12255;

		try {
			const uint64_t long_port = parseUlong(portEntry.get_text());
			if (UINT16_MAX < long_port)
				throw std::invalid_argument("Port too large");
		} catch (const std::invalid_argument &) {}

		signal_submit_.emit(hostnameEntry.get_text(), port);
	}
}
