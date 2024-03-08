#pragma once

#include "ui/tab/Tab.h"

#include <string>
#include <vte/vte.h>

namespace Game3 {
	class MainWindow;

	class LogTab: public Tab {
		public:
			LogTab() = delete;
			LogTab(Gtk::Notebook &);

			Gtk::Widget & getWidget() override { return scrolled; }
			std::string getName() override { return "Log"; }
			void reset(const std::shared_ptr<ClientGame> &) override;
			void print(std::string_view);

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::Widget *terminal = nullptr;
			VteTerminal *vte = nullptr;
	};
}
