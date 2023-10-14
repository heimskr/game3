#pragma once

#include <gtkmm.h>

#include "ui/gtk/NumericEntry.h"

namespace Game3 {
	class CraftXDialog: public Gtk::MessageDialog {
		public:
			CraftXDialog(Gtk::Window &);

			inline auto signal_submit() const { return signal_submit_; }

		private:
			sigc::signal<void(int)> signal_submit_;
			Gtk::Label countLabel{"Number"};
			Gtk::SpinButton countSpin;

			void submit();
	};
}
