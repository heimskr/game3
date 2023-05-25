#include <iostream>

#include "entity/Merchant.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "game/Stonks.h"
#include "ui/MainWindow.h"
#include "ui/gtk/EntryDialog.h"
#include "ui/gtk/NumericEntry.h"
#include "ui/gtk/Util.h"
#include "ui/tab/MerchantTab.h"
#include "util/Util.h"

// There's a lot of code duplication here.
// Very sorry.

namespace Game3 {
	MerchantTab::MerchantTab(MainWindow &main_window): Tab(main_window.notebook), mainWindow(main_window) {
		vbox.append(playerGrid);
		vbox.append(hbox);
		vbox.append(merchantGrid);
		hbox.append(merchantLabel);
		merchantLabel.set_hexpand(true);
		scrolled.set_child(vbox);
		scrolled.set_vexpand(true);
		auto gmenu = Gio::Menu::create();
		gmenu->append("_Drop", "merchant_popup.drop");
		gmenu->append("D_iscard", "merchant_popup.discard");
		popoverMenu.set_menu_model(gmenu);

		auto group = Gio::SimpleActionGroup::create();
		group->add_action("drop", [this] {
			if (!lastMerchant)
				lastGame->player->inventory->drop(lastSlot);
		});
		group->add_action("discard", [this] {
			if (!lastMerchant)
				lastGame->player->inventory->erase(lastSlot);
		});

		mainWindow.insert_action_group("merchant_popup", group);
		popoverMenu.set_parent(mainWindow); // TODO: fix this silliness

		auto source = Gtk::DragSource::create();
		source->set_actions(Gdk::DragAction::MOVE);
		source->signal_prepare().connect([this, source](double x, double y) -> Glib::RefPtr<Gdk::ContentProvider> { // Does capturing `source` cause a memory leak?
			auto *item = playerGrid.pick(x, y);
			draggedSlot = -1;

			if (dynamic_cast<Gtk::Fixed *>(item->get_parent()))
				item = item->get_parent();

			if (auto *label = dynamic_cast<Gtk::Label *>(item)) {
				if (label->get_text().empty())
					return nullptr;
			} else if (!dynamic_cast<Gtk::Fixed *>(item))
				return nullptr;

			draggedSlot = widgetMap.at(item);

			Glib::ValueBase base;
			base.init(GTK_TYPE_WIDGET);
			return Gdk::ContentProvider::create(base);
		}, false);
		playerGrid.add_controller(source);

		auto target = Gtk::DropTarget::create(GTK_TYPE_WIDGET, Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &, double x, double y) {
			auto *destination = playerGrid.pick(x, y);

			if (destination != nullptr && destination != &playerGrid) {
				if (dynamic_cast<Gtk::Fixed *>(destination->get_parent()) != nullptr)
					destination = destination->get_parent();
				const Slot source_slot = draggedSlot;
				const Slot destination_slot = widgetMap.at(destination);
				mainWindow.game->player->inventory->swap(source_slot, destination_slot);
			}

			return true;
		}, false);
		playerGrid.add_controller(target);
	}

	void MerchantTab::onResize(const std::shared_ptr<ClientGame> &game) {
		if (gridWidth() != lastGridWidth)
			reset(game);
	}

	void MerchantTab::update(const std::shared_ptr<ClientGame> &) {

	}

	void MerchantTab::reset(const std::shared_ptr<ClientGame> &game) {
		if (!game)
			return;

		widgetMap.clear();

		removeChildren(playerGrid);
		removeChildren(merchantGrid);
		if (!merchantName.empty()) {
			merchantLabel.set_text(merchantName);
			merchantLabel.show();
		}

		playerWidgets.clear();
		merchantWidgets.clear();

		if (scrolled.get_width() == 0)
			return;

		const int grid_width = lastGridWidth = gridWidth();
		const int tile_size = scrolled.get_width() / (scrolled.get_width() / TILE_SIZE);

		auto populate = [&](Gtk::Grid &grid, Inventory &inventory, bool is_merchant) {
			auto &storage = inventory.getStorage();
			auto &widgets = is_merchant? merchantWidgets : playerWidgets;

			for (Slot slot = 0; slot < inventory.slotCount; ++slot) {
				const int row    = slot / grid_width;
				const int column = slot % grid_width;
				std::unique_ptr<Gtk::Widget> widget_ptr;

				if (storage.contains(slot)) {
					auto &stack = storage.at(slot);
					Glib::ustring label_text = stack.item->name;
					if (stack.count != 1)
						label_text += " \u00d7 " + std::to_string(stack.count);
					if (is_merchant) {
						const auto &merchant = *std::dynamic_pointer_cast<Merchant>(inventory.getOwner());
						const MoneyCount for_one = totalBuyPrice(merchant, stack.withCount(1));
						const MoneyCount for_all = totalBuyPrice(merchant, stack);
						label_text += "\n(" + std::to_string(for_one) + " for one, " + std::to_string(for_all) + " for all)";
					}
					auto fixed_ptr = std::make_unique<Gtk::Fixed>();
					auto image_ptr = std::make_unique<Gtk::Image>(inventory.getImage(*game, slot));
					auto label_ptr = std::make_unique<Gtk::Label>(std::to_string(stack.count));
					label_ptr->set_xalign(1.f);
					label_ptr->set_yalign(1.f);
					auto &fixed = *fixed_ptr;
					fixed.put(*image_ptr, 0, 0);
					fixed.put(*label_ptr, 0, 0);
					fixed.set_tooltip_text(label_text);
					auto left_click = Gtk::GestureClick::create();
					auto right_click = Gtk::GestureClick::create();
					left_click->set_button(1);
					right_click->set_button(3);
					left_click ->signal_pressed().connect([this, game, slot, is_merchant, &fixed](int n, double x, double y) { leftClick (game, &fixed, n, slot, is_merchant, x, y); });
					right_click->signal_pressed().connect([this, game, slot, is_merchant, &fixed](int n, double x, double y) { rightClick(game, &fixed, n, slot, is_merchant, x, y); });
					fixed.add_controller(left_click);
					fixed.add_controller(right_click);
					widget_ptr = std::move(fixed_ptr);
					image_ptr->set_size_request(tile_size - MerchantTab::TILE_MAGIC, tile_size - MerchantTab::TILE_MAGIC);
					label_ptr->set_size_request(tile_size - MerchantTab::TILE_MAGIC, tile_size - MerchantTab::TILE_MAGIC);
					widgets.push_back(std::move(image_ptr));
					widgets.push_back(std::move(label_ptr));
				} else
					widget_ptr = std::make_unique<Gtk::Label>("");

				if (auto label = dynamic_cast<Gtk::Label *>(widget_ptr.get())) {
					label->set_wrap(true);
					label->set_wrap_mode(Pango::WrapMode::CHAR);
				}

				widget_ptr->set_size_request(tile_size, tile_size);
				widget_ptr->add_css_class("item-slot");
				widget_ptr->set_data("slot", reinterpret_cast<void *>(slot));
				grid.attach(*widget_ptr, column, row);
				widgetMap.emplace(widget_ptr.get(), slot);
				widgets.push_back(std::move(widget_ptr));
			}
		};

		if (game->player->inventory)
			populate(playerGrid, *game->player->inventory, false);

		if (merchantInventory)
			populate(merchantGrid, *merchantInventory, true);
	}

	void MerchantTab::setMerchantInventory(const Glib::ustring &name, const std::shared_ptr<Inventory> &inventory, double price_multiplier) {
		merchantInventory = inventory;
		merchantName = name;
		priceMultiplier = price_multiplier;
		if (inventory)
			if (auto owner = inventory->weakOwner.lock())
				reset(owner->getRealm()->getGame().toClientPointer());
	}

	void MerchantTab::resetMerchantInventory() {
		removeChildren(merchantGrid);
		merchantLabel.hide();
		merchantLabel.set_text("");
		merchantWidgets.clear();
		merchantInventory.reset();
		merchantName.clear();
	}

	int MerchantTab::gridWidth() const {
		return scrolled.get_width() / (TILE_SIZE + 2 * TILE_MARGIN);
	}

	void MerchantTab::leftClick(const std::shared_ptr<ClientGame> &, Gtk::Widget *, int click_count, Slot slot, bool merchant, double, double) {
		mainWindow.onBlur();

		if (click_count % 2 == 0) {
			std::cout << "Clicked on slot " << slot << " in the " << (merchant? "merchant" : "player") << "'s inventory\n";
		}
	}

	void MerchantTab::rightClick(const std::shared_ptr<ClientGame> &game, Gtk::Widget *widget, int, Slot slot, bool merchant, double x, double y) {
		mainWindow.onBlur();

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
		lastMerchant = merchant;
		popoverMenu.popup();
	}

	Gtk::Widget & MerchantTab::getDraggedItem() {
		return *reinterpret_cast<Gtk::Widget *>(scrolled.get_data("dragged-item"));
	}
}
