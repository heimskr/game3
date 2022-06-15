#pragma once

#include <unordered_map>
#include <utility>

#include "ui/tab/Tab.h"

namespace Game3 {
	class MainWindow;

	class CraftingTab: public Tab {
		public:
			MainWindow &mainWindow;

			CraftingTab() = delete;
			CraftingTab(MainWindow &);

			CraftingTab(const CraftingTab &) = delete;
			CraftingTab(CraftingTab &&) = delete;

			CraftingTab & operator=(const CraftingTab &) = delete;
			CraftingTab & operator=(CraftingTab &&) = delete;

			Gtk::Widget & getWidget() override { return scrolled; }
			Glib::ustring getName() override { return "Crafting"; }
			void update(const std::shared_ptr<Game> &) override;
			void reset(const std::shared_ptr<Game> &) override;

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::Box vbox {Gtk::Orientation::VERTICAL};
			Gtk::PopoverMenu popoverMenu;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;

			/** We can't store state in a popover, so we have to store it here. */
			std::shared_ptr<Game> lastGame;
			size_t lastIndex = 0;

			void craftOne(const std::shared_ptr<Game> &, size_t index);
			void leftClick(const std::shared_ptr<Game> &, Gtk::Widget *, size_t index, int n, double x, double y);
			void rightClick(const std::shared_ptr<Game> &, Gtk::Widget *, size_t index, double x, double y);
	};
}
