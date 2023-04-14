#pragma once

#include <gtkmm.h>

#include "ui/gtk/NumericEntry.h"

namespace Game3 {
	class MainWindow;
	struct WorldGenParams;

	class NewGameDialog: public Gtk::MessageDialog {
		public:
			NewGameDialog(MainWindow &parent_);

			inline auto signal_submit() const { return signal_submit_; }

		private:
			MainWindow &mainWindow;
			sigc::signal<void(long, long, long, const WorldGenParams &)> signal_submit_;
			Gtk::Label seedLabel {"Seed"};
			Gtk::Label widthLabel {"Width"};
			Gtk::Label heightLabel {"Height"};
			Gtk::Label wetnessLabel {"Wetness"};
			Gtk::Label stoneLevelLabel {"Stone Level"};
			Gtk::Entry seedEntry;
			Gtk::Scale wetnessSlider;
			Gtk::Scale stoneLevelSlider;
			NumericEntry widthEntry, heightEntry;

			void submit();
	};
}
