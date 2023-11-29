#include "item/Item.h"
#include "ui/gtk/ItemSlot.h"

namespace Game3 {
	namespace {
		constexpr int TILE_SIZE = 64;
	}

	ItemSlot::ItemSlot() {
		label.set_xalign(1.f);
		label.set_yalign(1.f);
		label.set_expand(true);
		label.set_size_request(TILE_SIZE, TILE_SIZE);
		image.set_expand(true);
		image.set_size_request(TILE_SIZE, TILE_SIZE);
		durabilityBar.add_css_class("item-durability");
		add_css_class("item-slot");
		set_size_request(TILE_SIZE, TILE_SIZE);
		put(label, 0, 0);
		put(image, 0, 0);
	}

	void ItemSlot::reset() {
		set_tooltip_text({});
		label.set_text({});
		image.clear();
		remove(durabilityBar);
		durabilityVisible = false;
		isEmpty = true;
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

	bool ItemSlot::empty() const {
		return isEmpty;
	}

	void ItemSlot::setLeftClick(ClickFn click) {
		setClick(std::move(click), &ItemSlot::leftClick, &ItemSlot::leftGesture, 1);
	}

	void ItemSlot::setRightClick(ClickFn click) {
		setClick(std::move(click), &ItemSlot::rightClick, &ItemSlot::rightGesture, 3);
	}

	void ItemSlot::addDurabilityBar(double fraction) {
		durabilityBar.set_fraction(fraction);
		durabilityBar.set_size_request(TILE_SIZE, -1);
		if (!durabilityVisible) {
			durabilityVisible = true;
			put(durabilityBar, 0, 0);
		}
	}

	void ItemSlot::setClick(ClickFn new_click, ClickFn ItemSlot::*click, Glib::RefPtr<Gtk::GestureClick> ItemSlot::*gesture, int button) {
		if (this->*gesture)
			remove_controller(this->*gesture);

		this->*click = std::move(new_click);

		this->*gesture = Gtk::GestureClick::create();
		(this->*gesture)->set_button(button);
		(this->*gesture)->signal_released().connect([this, click, gesture](int n, double x, double y) {
			const auto mods = (this->*gesture)->get_current_event_state();
			(this->*click)(Modifiers{mods}, n, x, y);
		});

		add_controller(this->*gesture);
	}
}
