#include "entity/ClientPlayer.h"
#include "interface/ItemSlotParent.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/Item.h"
#include "packet/MoveSlotsPacket.h"
#include "ui/gtk/DragSource.h"
#include "ui/gtk/ItemSlot.h"

namespace Game3 {
	namespace {
		constexpr int TILE_SIZE = 64;
	}

	ItemSlot::ItemSlot(const ClientGamePtr &game, Slot slot_, ClientInventoryPtr inventory_, ItemSlotParent *parent_):
	weakGame(game), slot(slot_), inventory(std::move(inventory_)), parent(parent_) {
		set_hexpand(false);
		set_halign(Gtk::Align::START);
		label.set_xalign(1.f);
		label.set_yalign(1.f);
		label.set_expand(true);
		label.set_size_request(TILE_SIZE, TILE_SIZE);
		image.set_expand(true);
		image.set_size_request(TILE_SIZE, TILE_SIZE);
		durabilityBar.add_css_class("item-durability");
		add_css_class("item-slot");
		put(image, 0, 0);
		put(label, 0, 0);
		set_size_request(TILE_SIZE, TILE_SIZE);
		popoverMenu.set_parent(*this);

		dragSource = Gtk::DragSource::create();
		dragSource->set_actions(Gdk::DragAction::MOVE);
		dragSource->signal_prepare().connect([this](double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
			if (!inventory)
				return {};
			Glib::Value<DragSource> value;
			value.init(value.value_type());
			value.set({slot, inventory, inventory->index});
			return Gdk::ContentProvider::create(value);
		}, false);

		dragSource->signal_drag_begin().connect([this](const Glib::RefPtr<Gdk::Drag> &) {
			// This is what you have to do to disable the drag icon? Kinda silly.
			dragSource->set_icon(Gdk::Texture::create_for_pixbuf(Gdk::Pixbuf::create(Gdk::Colorspace::RGB, true, 8, 1, 1)), 0, 0);
		});

		auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &base, double, double) {
			if (base.gobj()->g_type != Glib::Value<DragSource>::value_type())
				return false;

			const auto &value = static_cast<const Glib::Value<DragSource> &>(base);
			const DragSource source = value.get();

			if (onDrop)
				return onDrop(source.getStack());

			if (!inventory)
				return false;

			if (ClientGamePtr game = weakGame.lock())
				game->player->send(MoveSlotsPacket(source.inventory->getOwner()->getGID(), inventory->getOwner()->getGID(), source.slot, slot, source.index, inventory->index));
			return true;
		}, false);

		rightGesture = Gtk::GestureClick::create();
		rightGesture->set_button(3);
		rightGesture->signal_released().connect([this](int, double x, double y) {
			if (parent)
				parent->slotClicked(slot, true, Modifiers{rightGesture->get_current_event_state()});
			if (empty() || !gmenu)
				return;
			const auto allocation = get_allocation();
			x += allocation.get_x();
			y += allocation.get_y();
			popoverMenu.set_has_arrow(true);
			popoverMenu.set_pointing_to({int(x), int(y), 1, 1});
			popoverMenu.set_menu_model(gmenu);
			popoverMenu.popup();
		});

		leftGesture = Gtk::GestureClick::create();
		leftGesture->set_button(1);
		leftGesture->signal_released().connect([this](int n, double x, double y) {
			const Modifiers mods{leftGesture->get_current_event_state()};

			if (parent)
				parent->slotClicked(slot, false, mods);

			if (leftClick)
				leftClick(mods, n, x, y);
		});

		add_controller(dragSource);
		add_controller(target);
		add_controller(leftGesture);
		add_controller(rightGesture);
	}

	void ItemSlot::setStack(ItemStack stack) {
		image.set(stack.getImage());

		if (stack.count == ItemCount(-1))
			label.set_text({});
		else
			label.set_text(std::to_string(stack.count));

		Glib::ustring tooltip = stack.item->getTooltip(stack);

		if (stack.count != 1 && stack.count != ItemCount(-1))
			tooltip += " \u00d7 " + std::to_string(stack.count);

		if (stack.hasDurability()) {
			const nlohmann::json &durability = stack.data.at("durability");
			tooltip += "\n(" + std::to_string(durability.at(0).get<Durability>()) + '/' + std::to_string(durability.at(1).get<Durability>()) + ')';
			addDurabilityBar(stack.getDurabilityFraction());
		} else if (durabilityVisible) {
			durabilityVisible = false;
			remove(durabilityBar);
		}

		set_tooltip_text(tooltip);
		storedStack.emplace(std::move(stack));
	}

	void ItemSlot::reset() {
		set_tooltip_text({});
		label.set_text({});
		image.clear();
		if (durabilityVisible) {
			remove(durabilityBar);
			durabilityVisible = false;
		}
		storedStack.reset();
	}

	bool ItemSlot::empty() const {
		return !storedStack.has_value();
	}

	void ItemSlot::setLeftClick(ClickFn click) {
		leftClick = std::move(click);
	}

	void ItemSlot::setGmenu(Glib::RefPtr<Gio::Menu> new_gmenu) {
		gmenu = std::move(new_gmenu);
	}

	void ItemSlot::setInventory(std::shared_ptr<ClientInventory> new_inventory) {
		inventory = new_inventory;
	}

	void ItemSlot::addDurabilityBar(double fraction) {
		durabilityBar.set_fraction(fraction);
		durabilityBar.set_size_request(TILE_SIZE, -1);
		if (!durabilityVisible) {
			durabilityVisible = true;
			put(durabilityBar, 0, 0);
		}
	}
}
