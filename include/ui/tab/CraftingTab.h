#pragma once

#include "ui/tab/GTKTab.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Game3 {
	class MainWindow;

	class CraftingTab: public GTKTab {
		public:
			MainWindow &mainWindow;

			CraftingTab(MainWindow &);

			~CraftingTab() override;

			Gtk::Widget & getWidget() override { return scrolled; }
			std::string getName() override { return "Crafting"; }
			void update(const std::shared_ptr<ClientGame> &) override;
			void reset(const std::shared_ptr<ClientGame> &) override;

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Glib::RefPtr<Gio::Menu> gmenu;
			Gtk::PopoverMenu popoverMenu;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;

			/** We can't store state in a popover, so we have to store it here. */
			std::shared_ptr<ClientGame> lastGame;
			size_t lastRegistryID = 0;

			void craftOne(const std::shared_ptr<ClientGame> &, size_t registry_id);
			void craftX(const std::shared_ptr<ClientGame> &, size_t registry_id);
			void craftAll(const std::shared_ptr<ClientGame> &, size_t registry_id);

			void leftClick(const std::shared_ptr<ClientGame> &, Gtk::Widget *, size_t registry_id, int n, double x, double y);
			void rightClick(const std::shared_ptr<ClientGame> &, Gtk::Widget *, size_t registry_id, double x, double y);

			void reconfigurePopover();
	};
}
