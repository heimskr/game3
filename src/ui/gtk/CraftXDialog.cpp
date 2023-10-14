#include "ui/gtk/CraftXDialog.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	CraftXDialog::CraftXDialog(Gtk::Window &parent_): Gtk::MessageDialog(parent_, "Craft X", false, Gtk::MessageType::QUESTION, Gtk::ButtonsType::OK_CANCEL, true) {
		auto *area = get_message_area();
		countLabel.set_halign(Gtk::Align::START);
		countSpin.set_adjustment(Gtk::Adjustment::create(1., 1., 1920., 1., 64.));
		// countSpin.set_value(1);
		countSpin.set_digits(0);

		// hostnameEntry.signal_activate().connect(sigc::mem_fun(*this, &CraftXDialog::submit));
		area->append(countLabel);
		area->append(countSpin);

		// add_button("C_onnect", Gtk::ResponseType::OK);
		int width, height;
		get_default_size(width, height);
		if (width < 200)
			set_default_size(200, height);

		signal_response().connect([this](int response) {
			if (response == Gtk::ResponseType::OK)
				submit();
			else
				hide();
		});

		set_deletable(false);
	}

	void CraftXDialog::submit() {
		hide();
		signal_submit_.emit(countSpin.get_value_as_int());
	}
}
