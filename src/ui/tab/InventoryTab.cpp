#include <iostream>

#include "game/Game.h"
#include "ui/MainWindow.h"
#include "ui/gtk/EntryDialog.h"
#include "ui/gtk/NumericEntry.h"
#include "ui/gtk/Util.h"
#include "ui/tab/InventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	InventoryTab::InventoryTab(MainWindow &main_window): mainWindow(main_window) {
		scrolled.set_child(grid);
		scrolled.set_vexpand(true);
		auto gmenu = Gio::Menu::create();
		gmenu->append("_Drop", "inventory_popup.drop");
		gmenu->append("D_iscard", "inventory_popup.discard");
		popoverMenu.set_menu_model(gmenu);

		auto group = Gio::SimpleActionGroup::create();
		group->add_action("drop", [this] {
			lastGame->player->inventory.drop(lastSlot);
		});
		group->add_action("discard", [this] { std::cout << "discard\n"; });

		mainWindow.insert_action_group("inventory_popup", group);
		popoverMenu.set_parent(mainWindow); // TODO: fix this silliness
	}

	void InventoryTab::onFocus() {
		// if (!mainWindow.game)
		// 	return;
	}

	void InventoryTab::onBlur() {
		// discardButton.reset();
	}

	void InventoryTab::update(const std::shared_ptr<Game> &) {

	}

	void InventoryTab::reset(const std::shared_ptr<Game> &game) {
		if (!game)
			return;

		removeChildren(grid);

		const int grid_width = grid.get_width() / TILE_SIZE;
		const int tile_size  = grid.get_width() / grid_width; // Some silliness to get things truncated. Or something.
		const auto &inventory = game->player->inventory;
		const auto &storage = inventory.getStorage();
		gridWidgets.clear();

		for (Slot slot = 0; slot < inventory.slotCount; ++slot) {
			const int row    = slot / grid_width;
			const int column = slot % grid_width;
			if (storage.contains(slot)) {
				const auto &stack = storage.at(slot);
				auto label_ptr = std::make_unique<Gtk::Label>(stack.item->name);
				auto &label = *label_ptr;
				label.set_size_request(tile_size, tile_size);
				grid.attach(label, column, row);
				auto left_click = Gtk::GestureClick::create();
				auto right_click = Gtk::GestureClick::create();
				left_click->set_button(1);
				right_click->set_button(3);
				left_click->signal_pressed().connect([this, game, slot, &label](int n, double x, double y) { leftClick(game, label, n, slot, x, y); });
				right_click->signal_pressed().connect([this, game, slot, &label](int n, double x, double y) { rightClick(game, label, n, slot, x, y); });
				label.add_controller(left_click);
				label.add_controller(right_click);
				gridWidgets.push_back(std::move(label_ptr));
			} else {
				auto label_ptr = std::make_unique<Gtk::Label>("");
				label_ptr->set_size_request(tile_size, tile_size);
				grid.attach(*label_ptr, column, row);
				gridWidgets.push_back(std::move(label_ptr));
			}
		}
	}
	
	void InventoryTab::leftClick(const std::shared_ptr<Game> &, Gtk::Label &, int, Slot, double, double) {
		
	}

	void InventoryTab::rightClick(const std::shared_ptr<Game> &game, Gtk::Label &label, int, Slot slot, double x, double y) {
		Gtk::Widget *widget = &label;

		do {
			const auto allocation = widget->get_allocation();
			x += allocation.get_x();
			y += allocation.get_y();
			widget = widget->get_parent();
		} while (widget);

		popoverMenu.set_has_arrow(true);
		popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
		lastGame = game;
		lastSlot = slot;
		popoverMenu.popup();
	}
}
