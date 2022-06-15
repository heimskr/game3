#include <iostream>

#include "game/Game.h"
#include "game/Inventory.h"
#include "ui/MainWindow.h"
#include "ui/gtk/EntryDialog.h"
#include "ui/gtk/NumericEntry.h"
#include "ui/gtk/Util.h"
#include "ui/tab/CraftingTab.h"
#include "util/Util.h"

namespace Game3 {
	CraftingTab::CraftingTab(MainWindow &main_window): Tab(main_window.notebook), mainWindow(main_window) {
		scrolled.set_child(vbox);
		scrolled.set_vexpand(true);

		auto gmenu = Gio::Menu::create();
		gmenu->append("Craft _1", "crafting_popup.craft_one");
		gmenu->append("Craft _X", "crafting_popup.craft_x");
		gmenu->append("Craft _All", "crafting_popup.craft_all");
		popoverMenu.set_menu_model(gmenu);

		auto group = Gio::SimpleActionGroup::create();
		group->add_action("craft_one", [this] { std::cout << "one\n"; });
		group->add_action("craft_x",   [this] { std::cout <<   "x\n"; });
		group->add_action("craft_all", [this] { std::cout << "all\n"; });

		mainWindow.insert_action_group("crafting_popup", group);
		popoverMenu.set_parent(mainWindow); // TODO: fix this silliness
	}

	void CraftingTab::update(const std::shared_ptr<Game> &) {}

	void CraftingTab::reset(const std::shared_ptr<Game> &game) {
		if (!game)
			return;

		lastGame = game;

		removeChildren(hbox);
		widgets.clear();

		size_t index = 0;
		auto &inventory = *game->player->inventory;
		for (const auto &recipe: game->recipes) {
			if (inventory.canCraft(recipe)) {
				auto hbox = std::make_unique<Gtk::Box>(Gtk::Orientation::HORIZONTAL);

				widgets.push_back(std::move(hbox));
			}

			++index;
		}
	}

	void CraftingTab::rightClick(const std::shared_ptr<Game> &game, Gtk::Widget *widget, size_t index, double x, double y) {
		do {
			const auto allocation = widget->get_allocation();
			x += allocation.get_x();
			y += allocation.get_y();
			widget = widget->get_parent();
		} while (widget);

		popoverMenu.set_has_arrow(true);
		popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
		lastGame = game;
		lastIndex = index;
		popoverMenu.popup();
	}
}
