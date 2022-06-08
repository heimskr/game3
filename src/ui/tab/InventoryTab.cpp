#include <iostream>

#include "game/Game.h"
#include "ui/MainWindow.h"
#include "ui/gtk/EntryDialog.h"
#include "ui/gtk/NumericEntry.h"
#include "ui/gtk/Util.h"
#include "ui/tab/InventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	InventoryTab::InventoryTab(MainWindow &main_window): Tab(main_window.notebook), mainWindow(main_window) {
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
		group->add_action("discard", [this] {
			lastGame->player->inventory.erase(lastSlot);
		});

		mainWindow.insert_action_group("inventory_popup", group);
		popoverMenu.set_parent(mainWindow); // TODO: fix this silliness

		auto source = Gtk::DragSource::create();
		source->set_actions(Gdk::DragAction::MOVE);
		source->signal_prepare().connect([this, source](double x, double y) -> Glib::RefPtr<Gdk::ContentProvider> { // Does capturing `source` cause a memory leak?
			auto *item = grid.pick(x, y);
			grid.set_data("dragged-item", nullptr);
			if (auto *label = dynamic_cast<Gtk::Label *>(item)) {
				if (label->get_text().empty())
					return nullptr;
			} else
				return nullptr;
			grid.set_data("dragged-item", item);
			Glib::ValueBase base;
			base.init(GTK_TYPE_WIDGET);
			return Gdk::ContentProvider::create(base);
		}, false);
		grid.add_controller(source);

		auto target = Gtk::DropTarget::create(GTK_TYPE_WIDGET, Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &, double x, double y) {
			auto *destination = grid.pick(x, y);
			Gtk::Label *label = nullptr;

			if (destination && destination != &grid && (label = dynamic_cast<Gtk::Label *>(destination)) != nullptr) {
				const Slot source_slot = reinterpret_cast<intptr_t>(reinterpret_cast<Gtk::Label *>(grid.get_data("dragged-item"))->get_data("slot"));
				const Slot destination_slot = reinterpret_cast<intptr_t>(label->get_data("slot"));
				mainWindow.game->player->inventory.swap(source_slot, destination_slot);
			}

			return true;
		}, false);
		grid.add_controller(target);
	}

	void InventoryTab::onResize(const std::shared_ptr<Game> &game) {
		if (gridWidth() != lastGridWidth)
			reset(game);
	}

	void InventoryTab::update(const std::shared_ptr<Game> &) {

	}

	void InventoryTab::reset(const std::shared_ptr<Game> &game) {
		if (!game)
			return;

		removeChildren(grid);

		const int grid_width = lastGridWidth = gridWidth();
		const int tile_size  = grid.get_width() / (grid.get_width() / TILE_SIZE);
		const auto &inventory = game->player->inventory;
		const auto &storage = inventory.getStorage();
		gridWidgets.clear();

		for (Slot slot = 0; slot < inventory.slotCount; ++slot) {
			const int row    = slot / grid_width;
			const int column = slot % grid_width;
			std::unique_ptr<Gtk::Label> label_ptr;

			if (storage.contains(slot)) {
				const auto &stack = storage.at(slot);
				Glib::ustring label_text = stack.item->name;
				if (stack.count != 1)
					label_text += "\n\u00d7 " + std::to_string(stack.count);
				label_ptr = std::make_unique<Gtk::Label>(label_text);
				auto &label = *label_ptr;
				auto left_click = Gtk::GestureClick::create();
				auto right_click = Gtk::GestureClick::create();
				left_click->set_button(1);
				right_click->set_button(3);
				left_click->signal_pressed().connect([this, game, slot, &label](int n, double x, double y) { leftClick(game, label, n, slot, x, y); });
				right_click->signal_pressed().connect([this, game, slot, &label](int n, double x, double y) { rightClick(game, label, n, slot, x, y); });
				label.add_controller(left_click);
				label.add_controller(right_click);
			} else
				label_ptr = std::make_unique<Gtk::Label>("");

			label_ptr->set_wrap(true);
			label_ptr->set_wrap_mode(Pango::WrapMode::CHAR);
			label_ptr->set_size_request(tile_size, tile_size);
			label_ptr->add_css_class("item-slot");
			label_ptr->set_data("slot", reinterpret_cast<void *>(slot));
			grid.attach(*label_ptr, column, row);
			gridWidgets.push_back(std::move(label_ptr));
		}
	}

	int InventoryTab::gridWidth() const {
		return scrolled.get_width() / (TILE_SIZE + 2 * TILE_MARGIN);
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
