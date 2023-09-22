#pragma once

#include <gtkmm.h>

namespace Game3 {
	namespace {
		constexpr const char *CONNECTION_SUCCESS_MESSAGE = "Connection established.\n\nUse Ctrl-C to send commands.";
	}

	class ConnectionSuccessDialog: public Gtk::MessageDialog {
		public:
			ConnectionSuccessDialog(Gtk::Window &parent);

			inline sigc::signal<void(bool)> signal_submit() const { return signal_submit_; }

		private:
			Gtk::Label label{"Use Ctrl-C to send commands."};
			sigc::signal<void(bool)> signal_submit_;
			Gtk::CheckButton checkButton{"Don't show again"};

			void submit();
	};
}
