#pragma once

#include <gtkmm.h>

namespace Game3 {
	class MainWindow;
	struct WorldGenParams;

	class NewGameDialog: public Gtk::MessageDialog {
		public:
			NewGameDialog(MainWindow &parent_);

			inline auto signal_submit() const { return signal_submit_; }

		private:
			MainWindow &mainWindow;
			sigc::signal<void(long, const WorldGenParams &)> signal_submit_;
			Gtk::Label seedLabel {"Seed"};
			Gtk::Label wetnessLabel {"Wetness"};
			Gtk::Label stoneLevelLabel {"Stone Level"};
			Gtk::Label forestLabel {"Forest Threshold"};
			Gtk::Label antiforestLabel {"Antiforest Threshold"};
			Gtk::Entry seedEntry;
			Gtk::Scale wetnessSlider;
			Gtk::Scale stoneLevelSlider;
			Gtk::Scale forestSlider;
			Gtk::Scale antiforestSlider;

			void submit();
	};
}
