#include "ui/gtk/NewGameDialog.h"
#include "ui/MainWindow.h"
#include "util/Util.h"

namespace Game3 {
	NewGameDialog::NewGameDialog(MainWindow &parent_): Gtk::Dialog("New Game", parent_), mainWindow(parent_) {
		auto *area = get_content_area();
		seedLabel.set_halign(Gtk::Align::START);
		widthLabel.set_halign(Gtk::Align::START);
		heightLabel.set_halign(Gtk::Align::START);
		area->set_orientation(Gtk::Orientation::VERTICAL);
		area->set_margin(5);
		seedLabel.set_margin(5);
		widthLabel.set_margin(5);
		heightLabel.set_margin(5);
		seedEntry.set_text("666");
		widthEntry.set_text("256");
		heightEntry.set_text("256");
		seedEntry.signal_activate().connect(sigc::mem_fun(*this, &NewGameDialog::submit));
		widthEntry.signal_activate().connect(sigc::mem_fun(*this, &NewGameDialog::submit));
		heightEntry.signal_activate().connect(sigc::mem_fun(*this, &NewGameDialog::submit));
		area->append(seedLabel);
		area->append(seedEntry);
		area->append(widthLabel);
		area->append(widthEntry);
		area->append(heightLabel);
		area->append(heightEntry);
		add_button("_Cancel", Gtk::ResponseType::CANCEL);
		add_button("Cr_eate", Gtk::ResponseType::OK);
		int width, height;
		set_modal(true);
		get_default_size(width, height);
		set_default_size(300, height);
		signal_response().connect([this](int response) {
			if (response == Gtk::ResponseType::OK)
				submit();
			else
				hide();
		});
		set_deletable(false);
	}

	void NewGameDialog::submit() {
		hide();

		long seed = 0, width = 0, height = 0;
		auto show_error = [this](const std::string &message) { mainWindow.queue([this, message] { mainWindow.error(message); }); };

		try {
			seed = parseLong(seedEntry.get_text());
		} catch (const std::invalid_argument &) {
			show_error("Invalid seed.");
			return;
		}

		try {
			width = parseLong(widthEntry.get_text());
		} catch (const std::invalid_argument &) {
			show_error("Invalid width.");
			return;
		}

		try {
			height = parseLong(heightEntry.get_text());
		} catch (const std::invalid_argument &) {
			show_error("Invalid height.");
			return;
		}

		signal_submit_.emit(seed, width, height);
	}
}
