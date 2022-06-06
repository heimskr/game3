#pragma once

#include <gtkmm.h>

namespace Game3 {
	class NumericEntry: public Gtk::Entry {
		public:
			NumericEntry();
			sigc::signal<void()> signal_activate() const { return signal_activate_; }

		private:
			sigc::signal<void()> signal_activate_;
	};
}
