#include "ui/LogOverlay.h"

namespace Game3 {
	LogOverlay::LogOverlay() {
		terminal = Gtk::manage(Glib::wrap(GTK_WIDGET(vte_terminal_new()), false));
		terminal->set_expand(true);
		vte = VTE_TERMINAL(terminal->gobj());
		set_child(*terminal);
		set_vexpand(true);
	}

	void LogOverlay::reset() {
		vte_terminal_reset(vte, true, true);
	}

	void LogOverlay::print(std::string_view text) {
		const char *linefeed = "\r\n";
		size_t newline{};

		while ((newline = text.find('\n')) != std::string_view::npos) {
			vte_terminal_feed(vte, text.data(), newline);
			vte_terminal_feed(vte, linefeed, 2);
			text.remove_prefix(newline + 1);
		}

		vte_terminal_feed(vte, text.data(), text.size());
	}
}
