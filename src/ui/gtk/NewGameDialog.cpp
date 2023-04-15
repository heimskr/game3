#include "ui/gtk/NewGameDialog.h"
#include "ui/MainWindow.h"
#include "util/Util.h"
#include "worldgen/WorldGen.h"

namespace Game3 {
	NewGameDialog::NewGameDialog(MainWindow &parent_): Gtk::MessageDialog(parent_, "New Game", false, Gtk::MessageType::OTHER, Gtk::ButtonsType::CANCEL, true), mainWindow(parent_) {
		auto *area = get_message_area();
		seedLabel.set_halign(Gtk::Align::START);
		widthLabel.set_halign(Gtk::Align::START);
		heightLabel.set_halign(Gtk::Align::START);
		wetnessLabel.set_halign(Gtk::Align::START);
		stoneLevelLabel.set_halign(Gtk::Align::START);
		forestLabel.set_halign(Gtk::Align::START);
		antiforestLabel.set_halign(Gtk::Align::START);
		seedEntry.set_text("1621");
		widthEntry.set_text("256");
		heightEntry.set_text("256");
		seedEntry.signal_activate().connect(sigc::mem_fun(*this, &NewGameDialog::submit));
		widthEntry.signal_activate().connect(sigc::mem_fun(*this, &NewGameDialog::submit));
		heightEntry.signal_activate().connect(sigc::mem_fun(*this, &NewGameDialog::submit));
		const WorldGenParams params;
		wetnessSlider.set_digits(2);
		wetnessSlider.set_value_pos(Gtk::PositionType::RIGHT);
		wetnessSlider.set_range(-2.0, 2.0);
		wetnessSlider.set_value(params.wetness);
		wetnessSlider.set_draw_value();
		stoneLevelSlider.set_digits(2);
		stoneLevelSlider.set_value_pos(Gtk::PositionType::RIGHT);
		stoneLevelSlider.set_range(-2.0, 2.0);
		stoneLevelSlider.set_value(params.stoneLevel);
		stoneLevelSlider.set_draw_value();
		forestSlider.set_digits(2);
		forestSlider.set_value_pos(Gtk::PositionType::RIGHT);
		forestSlider.set_range(-1.0, 1.0);
		forestSlider.set_value(params.forestThreshold);
		forestSlider.set_draw_value();
		antiforestSlider.set_digits(2);
		antiforestSlider.set_value_pos(Gtk::PositionType::RIGHT);
		antiforestSlider.set_range(-1.0, 1.0);
		antiforestSlider.set_value(params.antiforestThreshold);
		antiforestSlider.set_draw_value();
		area->append(seedLabel);
		area->append(seedEntry);
		area->append(widthLabel);
		area->append(widthEntry);
		area->append(heightLabel);
		area->append(heightEntry);
		area->append(wetnessLabel);
		area->append(wetnessSlider);
		area->append(stoneLevelLabel);
		area->append(stoneLevelSlider);
		area->append(forestLabel);
		area->append(forestSlider);
		area->append(antiforestLabel);
		area->append(antiforestSlider);
		add_button("Cr_eate", Gtk::ResponseType::OK);
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

	void NewGameDialog::submit() {
		hide();

		long seed = 0, width = 0, height = 0;
		auto show_error = [this](const std::string &message) {
			mainWindow.queue([this, message] {
				mainWindow.error(message);
			});
		};

		try {
			seed = parseLong(seedEntry.get_text());
		} catch (const std::invalid_argument &) {
			seed = std::hash<std::string>()(seedEntry.get_text().raw());
			std::cout << "Seed: " << seedEntry.get_text() << " -> " << seed << '\n';
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

		signal_submit_.emit(seed, width, height, {
			.wetness = wetnessSlider.get_value(),
			.stoneLevel = stoneLevelSlider.get_value(),
			.forestThreshold = forestSlider.get_value(),
			.antiforestThreshold = antiforestSlider.get_value(),
		});
	}
}
