#pragma once

#include "ui/Modifiers.h"

#include <gtkmm.h>

#include <functional>
#include <memory>

namespace Game3 {
	class ClientGame;
	class ClientInventory;
	class ItemStack;

	class ItemSlot: public Gtk::Fixed {
		public:
			using ClickFn = std::function<void(Modifiers, int, double, double)>;

			ItemSlot() = delete;
			ItemSlot(std::shared_ptr<ClientGame>, Slot, std::shared_ptr<ClientInventory>);

			void setStack(const ItemStack &);
			void reset();
			bool empty() const;
			void setLeftClick(ClickFn);
			void setRightClick(ClickFn);

		private:
			std::shared_ptr<ClientGame> game;
			Slot slot;
			std::shared_ptr<ClientInventory> inventory;
			bool isEmpty = true;
			bool durabilityVisible = false;

			Gtk::Image image;
			Gtk::Label label;
			Gtk::ProgressBar durabilityBar;

			ClickFn leftClick;
			ClickFn rightClick;

			Glib::RefPtr<Gtk::GestureClick> leftGesture;
			Glib::RefPtr<Gtk::GestureClick> rightGesture;

			void addDurabilityBar(double fraction);
			void setClick(ClickFn, ClickFn ItemSlot::*, Glib::RefPtr<Gtk::GestureClick> ItemSlot::*, int button);
	};
}
