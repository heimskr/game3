#include "entity/ClientPlayer.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "item/Item.h"
#include "packet/MoveSlotsPacket.h"
#include "ui/gtk/ItemSlot.h"
#include "ui/gtk/UITypes.h"

namespace Game3 {
	namespace {
		constexpr int TILE_SIZE = 64;
	}

	ItemSlot::ItemSlot(std::shared_ptr<ClientGame> game_, Slot slot_, std::shared_ptr<ClientInventory> inventory_):
	game(std::move(game_)), slot(slot_), inventory(std::move(inventory_)) {
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

		auto source = Gtk::DragSource::create();
		source->set_actions(Gdk::DragAction::MOVE);
		source->signal_prepare().connect([this](double, double) -> Glib::RefPtr<Gdk::ContentProvider> {
			Glib::Value<DragSource> value;
			value.init(value.value_type());
			value.set({slot, inventory, inventory->index});
			return Gdk::ContentProvider::create(value);
		}, false);

		auto target = Gtk::DropTarget::create(Glib::Value<DragSource>::value_type(), Gdk::DragAction::MOVE);
		target->signal_drop().connect([this](const Glib::ValueBase &base, double, double) {
			if (base.gobj()->g_type != Glib::Value<DragSource>::value_type())
				return false;

			const auto &value = static_cast<const Glib::Value<DragSource> &>(base);
			const DragSource source = value.get();
			game->player->send(MoveSlotsPacket(source.inventory->getOwner()->getGID(), inventory->getOwner()->getGID(), source.slot, slot, source.index, inventory->index));
			return true;
		}, false);

		auto right_click = Gtk::GestureClick::create();
		right_click->set_button(3);
		right_click->signal_released().connect([this](int, double x, double y) {
			INFO("right clicked. failure conditions: " << empty() << ", " << !gmenu);
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

		add_controller(source);
		add_controller(target);
		add_controller(right_click);
	}

	void ItemSlot::setStack(const ItemStack &stack) {
		image.set(stack.getImage());
		label.set_text(std::to_string(stack.count));

		Glib::ustring tooltip = stack.item->getTooltip(stack);

		if (stack.count != 1)
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
		isEmpty = false;
	}

	void ItemSlot::reset() {
		set_tooltip_text({});
		label.set_text({});
		image.clear();
		if (durabilityVisible) {
			remove(durabilityBar);
			durabilityVisible = false;
		}
		isEmpty = true;
	}

	bool ItemSlot::empty() const {
		return isEmpty;
	}

	void ItemSlot::setLeftClick(ClickFn click) {
		if (leftGesture)
			remove_controller(leftGesture);

		leftClick = std::move(click);

		leftGesture = Gtk::GestureClick::create();
		leftGesture->set_button(1);
		leftGesture->signal_released().connect([this](int n, double x, double y) {
			const Gdk::ModifierType mods = leftGesture->get_current_event_state();
			leftClick(Modifiers{mods}, n, x, y);
		});

		add_controller(leftGesture);
	}

	void ItemSlot::setGmenu(Glib::RefPtr<Gio::Menu> new_gmenu) {
		gmenu = std::move(new_gmenu);
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
