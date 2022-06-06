#pragma once

#include <gtkmm.h>

#include "ui/gtk/NumericEntry.h"

namespace Game3 {
	class MainWindow;

	class NewGameDialog: public Gtk::Dialog {
		public:
			NewGameDialog(MainWindow &parent_);

			sigc::signal<void(long, long, long)> signal_submit() const { return signal_submit_; }

		private:
			MainWindow &mainWindow;
			sigc::signal<void(long, long, long)> signal_submit_;
			Gtk::Label seedLabel {"Seed"}, widthLabel {"Width"}, heightLabel {"Height"};
			NumericEntry seedEntry, widthEntry, heightEntry;

			void submit();
	};
}
