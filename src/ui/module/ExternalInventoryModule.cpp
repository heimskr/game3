#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "packet/MoveSlotsPacket.h"
#include "ui/gtk/UITypes.h"
#include "ui/gtk/Util.h"
#include "ui/module/ExternalInventoryModule.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	ExternalInventoryModule::ExternalInventoryModule(std::shared_ptr<ClientGame> game_, std::shared_ptr<ClientInventory> inventory_):
	game(std::move(game_)),
	inventory(std::move(inventory_)) {
		assert(inventory);
		label.set_hexpand();
		grid.set_hexpand();
		hbox.append(label);
		vbox.append(hbox);
		vbox.append(grid);

		gmenu = Gio::Menu::create();
		gmenu->append("_Drop", "inventory_popup.drop");
		gmenu->append("D_iscard", "inventory_popup.discard");

		popoverMenu.set_parent(vbox);

		auto source = Gtk::DragSource::create();
		source->set_actions(Gdk::DragAction::MOVE);
		source->signal_prepare().connect([this, source](double x, double y) -> Glib::RefPtr<Gdk::ContentProvider> { // Does capturing `source` cause a memory leak?
			auto *item = grid.pick(x, y);

			if (dynamic_cast<Gtk::Fixed *>(item->get_parent()))
				item = item->get_parent();

			if (auto *label = dynamic_cast<Gtk::Label *>(item)) {
				if (label->get_text().empty())
					return nullptr;
			} else if (!dynamic_cast<Gtk::Fixed *>(item))
				return nullptr;

			Glib::Value<DragSource> value;
			value.init(value.value_type());
			value.set({widgetMap.at(item), inventory});
			return Gdk::ContentProvider::create(value);
		}, false);

		auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &base, double x, double y) {
			if (base.gobj()->g_type != Glib::Value<DragSource>::value_type())
				return false;

			const auto &value = static_cast<const Glib::Value<DragSource> &>(base);
			auto *destination = grid.pick(x, y);

			if (destination != nullptr && destination != &grid) {
				if (dynamic_cast<Gtk::Fixed *>(destination->get_parent()))
					destination = destination->get_parent();

				const DragSource source = value.get();
				game->player->send(MoveSlotsPacket(source.inventory->getOwner()->getGID(), inventory->getOwner()->getGID(), source.slot, widgetMap.at(destination)));
			}

			return true;
		}, false);

		grid.add_controller(source);
		grid.add_controller(target);
	}

	Gtk::Widget & ExternalInventoryModule::getWidget() {
		return vbox;
	}

	void ExternalInventoryModule::reset() {
		widgetMap.clear();
		removeChildren(grid);
		widgets.clear();
		widgetsBySlot.clear();
		update();
	}

	void ExternalInventoryModule::update() {
		if (!name.empty()) {
			label.set_text(name);
			label.show();
		}

		populate();
	}

	void ExternalInventoryModule::onResize(int width) {
		tabWidth = width;
		update();
	}

	int ExternalInventoryModule::gridWidth() const {
		return tabWidth / (InventoryTab::TILE_SIZE + 2 * InventoryTab::TILE_MARGIN);
	}

	void ExternalInventoryModule::populate() {
		assert(inventory);
		auto &storage = inventory->getStorage();
		const int grid_width = gridWidth();
		const int tile_size  = InventoryTab::TILE_SIZE <= tabWidth? tabWidth / (tabWidth / InventoryTab::TILE_SIZE) : InventoryTab::TILE_SIZE;

		for (Slot slot = 0; slot < inventory->slotCount; ++slot) {
			const int row    = slot / grid_width;
			const int column = slot % grid_width;
			std::unique_ptr<Gtk::Widget> widget_ptr;

			if (storage.contains(slot)) {
				auto &stack = storage.at(slot);
				Glib::ustring label_text = stack.item->getTooltip(stack);
				if (stack.count != 1)
					label_text += " \u00d7 " + std::to_string(stack.count);
				if (stack.hasDurability())
					label_text += "\n(" + std::to_string(stack.data.at("durability").at(0).get<Durability>()) + "/" + std::to_string(stack.data.at("durability").at(1).get<Durability>()) + ")";
				auto fixed_ptr = std::make_unique<Gtk::Fixed>();
				auto image_ptr = std::make_unique<Gtk::Image>(inventory->getImage(*game, slot));
				auto label_ptr = std::make_unique<Gtk::Label>(std::to_string(stack.count));
				label_ptr->set_xalign(1.f);
				label_ptr->set_yalign(1.f);
				auto &fixed = *fixed_ptr;
				if (stack.hasDurability()) {
					auto progress_ptr = std::make_unique<Gtk::ProgressBar>();
					progress_ptr->set_fraction(stack.getDurabilityFraction());
					progress_ptr->add_css_class("item-durability");
					progress_ptr->set_size_request(tile_size - InventoryTab::TILE_MAGIC, -1);
					fixed.put(*progress_ptr, 0, 0);
					widgets.push_back(std::move(progress_ptr));
				}
				fixed.put(*image_ptr, 0, 0);
				fixed.put(*label_ptr, 0, 0);
				fixed.set_tooltip_text(label_text);
				widget_ptr = std::move(fixed_ptr);
				image_ptr->set_size_request(tile_size - InventoryTab::TILE_MAGIC, tile_size - InventoryTab::TILE_MAGIC);
				label_ptr->set_size_request(tile_size - InventoryTab::TILE_MAGIC, tile_size - InventoryTab::TILE_MAGIC);
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

			Gtk::Widget *old_widget = nullptr;
			if (auto iter = widgetsBySlot.find(slot); iter != widgetsBySlot.end()) {
				old_widget = iter->second;
				widgetMap.erase(iter->second);
			}
			widgetsBySlot[slot] = widget_ptr.get();

			auto right_click = Gtk::GestureClick::create();
			right_click->set_button(3);
			right_click->signal_pressed().connect([this, slot, widget = widget_ptr.get()](int n, double x, double y) {
				rightClick(widget, n, slot, x, y);
			});

			widget_ptr->add_controller(right_click);

			widgetMap[widget_ptr.get()] = slot;
			if (old_widget != nullptr)
				grid.remove(*old_widget);
			grid.attach(*widget_ptr, column, row);
			widgets.push_back(std::move(widget_ptr));
		}
	}

	void ExternalInventoryModule::rightClick(Gtk::Widget *widget, int, Slot slot, double x, double y) {
		// mainWindow.onBlur();

		if (!inventory->contains(slot))
			return;

		const auto allocation = widget->get_allocation();
		x += allocation.get_x();
		y += allocation.get_y();

		popoverMenu.set_has_arrow(true);
		popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
		popoverMenu.set_menu_model(gmenu);
		lastSlot = slot;
		popoverMenu.popup();
	}
}
