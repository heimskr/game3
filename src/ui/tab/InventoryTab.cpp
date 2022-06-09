#include <iostream>

#include "game/Game.h"
#include "game/Inventory.h"
#include "ui/MainWindow.h"
#include "ui/gtk/EntryDialog.h"
#include "ui/gtk/NumericEntry.h"
#include "ui/gtk/Util.h"
#include "ui/tab/InventoryTab.h"
#include "util/Util.h"

namespace Game3 {
	InventoryTab::InventoryTab(MainWindow &main_window): Tab(main_window.notebook), mainWindow(main_window) {
		vbox.append(playerGrid);
		vbox.append(hbox);
		vbox.append(externalGrid);
		hbox.append(externalLabel);
		externalLabel.set_hexpand(true);
		scrolled.set_child(vbox);
		scrolled.set_vexpand(true);
		auto gmenu = Gio::Menu::create();
		gmenu->append("_Drop", "inventory_popup.drop");
		gmenu->append("D_iscard", "inventory_popup.discard");
		popoverMenu.set_menu_model(gmenu);

		auto group = Gio::SimpleActionGroup::create();
		group->add_action("drop", [this] {
			(lastExternal? externalInventory : lastGame->player->inventory)->drop(lastSlot);
		});
		group->add_action("discard", [this] {
			(lastExternal? externalInventory : lastGame->player->inventory)->erase(lastSlot);
		});

		mainWindow.insert_action_group("inventory_popup", group);
		popoverMenu.set_parent(mainWindow); // TODO: fix this silliness

		for (bool external: {false, true}) {
			auto source = Gtk::DragSource::create();
			source->set_actions(Gdk::DragAction::MOVE);
			source->signal_prepare().connect([this, source, external](double x, double y) -> Glib::RefPtr<Gdk::ContentProvider> { // Does capturing `source` cause a memory leak?
				auto *item = (external? externalGrid : playerGrid).pick(x, y);
				scrolled.set_data("dragged-item", nullptr);
				if (auto *label = dynamic_cast<Gtk::Label *>(item)) {
					if (label->get_text().empty())
						return nullptr;
				} else
					return nullptr;
				scrolled.set_data("dragged-item", item);
				Glib::ValueBase base;
				base.init(GTK_TYPE_WIDGET);
				return Gdk::ContentProvider::create(base);
			}, false);
			(external? externalGrid : playerGrid).add_controller(source);

			auto target = Gtk::DropTarget::create(GTK_TYPE_WIDGET, Gdk::DragAction::MOVE);
			target->signal_drop().connect([this, external](const Glib::ValueBase &, double x, double y) {
				auto &grid = external? externalGrid : playerGrid;
				auto *destination = grid.pick(x, y);
				Gtk::Label *label = nullptr;

				if (destination && destination != &grid && (label = dynamic_cast<Gtk::Label *>(destination)) != nullptr) {
					auto &dragged = getDraggedItem();
					const Slot source_slot      = reinterpret_cast<intptr_t>(dragged.get_data("slot"));
					const Slot destination_slot = reinterpret_cast<intptr_t>(label->get_data("slot"));
					const bool from_external    = reinterpret_cast<intptr_t>(dragged.get_data("external"));
					const bool to_external      = reinterpret_cast<intptr_t>(label->get_data("external"));
					auto &player_inventory = *mainWindow.game->player->inventory;

					if (from_external) {
						if (to_external) {
							externalInventory->swap(source_slot, destination_slot);
						} else {
							const ItemStack *source = (*externalInventory)[source_slot];
							if (!source) {
								std::cerr << "Warning: slot " << source_slot << " not present in external inventory\n";
								return true;
							}

							if (player_inventory.canStore(*source)) {
								player_inventory.add(*source, destination_slot);
								externalInventory->erase(source_slot);
							}
						}
					} else {
						if (to_external) {
							const ItemStack *source = player_inventory[source_slot];
							if (!source) {
								std::cerr << "Warning: slot " << source_slot << " not present in player inventory\n";
								return true;
							}

							if (externalInventory->canStore(*source)) {
								externalInventory->add(*source, destination_slot);
								player_inventory.erase(source_slot);
							}
						} else
							player_inventory.swap(source_slot, destination_slot);
					}
				}

				return true;
			}, false);
			(external? externalGrid : playerGrid).add_controller(target);
		}
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

		removeChildren(playerGrid);
		removeChildren(externalGrid);
		if (!externalName.empty()) {
			externalLabel.set_text(externalName);
			externalLabel.show();
		}

		playerWidgets.clear();
		externalWidgets.clear();

		const int grid_width = lastGridWidth = gridWidth();
		const int tile_size  = playerGrid.get_width() / (playerGrid.get_width() / TILE_SIZE);

		auto populate = [&](Gtk::Grid &grid, const Inventory &inventory, bool external) {
			const auto &storage = inventory.getStorage();

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
					left_click ->signal_pressed().connect([this, game, slot, external, &label](int n, double x, double y) { leftClick (game, label, n, slot, external, x, y); });
					right_click->signal_pressed().connect([this, game, slot, external, &label](int n, double x, double y) { rightClick(game, label, n, slot, external, x, y); });
					label.add_controller(left_click);
					label.add_controller(right_click);
				} else
					label_ptr = std::make_unique<Gtk::Label>("");

				label_ptr->set_wrap(true);
				label_ptr->set_wrap_mode(Pango::WrapMode::CHAR);
				label_ptr->set_size_request(tile_size, tile_size);
				label_ptr->add_css_class("item-slot");
				label_ptr->set_data("slot", reinterpret_cast<void *>(slot));
				label_ptr->set_data("external", reinterpret_cast<void *>(external));
				grid.attach(*label_ptr, column, row);
				(external? externalWidgets : playerWidgets).push_back(std::move(label_ptr));
			}
		};

		if (game->player->inventory)
			populate(playerGrid, *game->player->inventory, false);

		if (externalInventory)
			populate(externalGrid, *externalInventory, true);
	}

	void InventoryTab::setExternalInventory(const Glib::ustring &name, const std::shared_ptr<Inventory> &inventory) {
		externalInventory = inventory;
		externalName = name;
		if (inventory)
			if (auto owner = inventory->owner.lock())
				reset(owner->getRealm()->getGame().shared_from_this());
	}

	void InventoryTab::resetExternalInventory() {
		removeChildren(externalGrid);
		externalLabel.hide();
		externalLabel.set_text("");
		externalWidgets.clear();
		externalInventory.reset();
		externalName.clear();
	}

	int InventoryTab::gridWidth() const {
		return scrolled.get_width() / (TILE_SIZE + 2 * TILE_MARGIN);
	}

	void InventoryTab::leftClick(const std::shared_ptr<Game> &, Gtk::Label &, int, Slot, bool, double, double) {
		
	}

	void InventoryTab::rightClick(const std::shared_ptr<Game> &game, Gtk::Label &label, int, Slot slot, bool external, double x, double y) {
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
		lastExternal = external;
		popoverMenu.popup();
	}

	Gtk::Label & InventoryTab::getDraggedItem() {
		return *reinterpret_cast<Gtk::Label *>(scrolled.get_data("dragged-item"));
	}
}
