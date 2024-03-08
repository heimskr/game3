#include <iostream>

#include "game/Game.h"
#include "ui/tab/LogTab.h"

namespace Game3 {
	LogTab::LogTab(Gtk::Notebook &notebook_): Tab(notebook_) {
		terminal = Gtk::manage(Glib::wrap(GTK_WIDGET(vte_terminal_new()), false));
		terminal->set_expand(true);
		vte = VTE_TERMINAL(terminal->gobj());

		scrolled.set_child(*terminal);
		scrolled.set_vexpand(true);
	}

	void LogTab::reset(const std::shared_ptr<ClientGame> &) {
		vte_terminal_reset(vte, true, true);
	}

	void LogTab::print(std::string_view text) {
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
