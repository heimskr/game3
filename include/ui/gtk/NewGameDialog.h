#pragma once

#include <gtkmm.h>

#include "ui/gtk/NumericEntry.h"

namespace Game3 {
	class MainWindow;

	class NewGameDialog: public Gtk::MessageDialog {
		public:
			NewGameDialog(MainWindow &parent_);

			inline auto signal_submit() const { return signal_submit_; }

		private:
			MainWindow &mainWindow;
			sigc::signal<void(long, long, long, double)> signal_submit_;
			Gtk::Label seedLabel {"Seed"}, widthLabel {"Width"}, heightLabel {"Height"}, wetnessLabel {"Wetness"};
			Gtk::Entry seedEntry;
			Gtk::Scale wetnessSlider;
			NumericEntry widthEntry, heightEntry;

			void submit();
	};
}
