#pragma once

#include "ui/Modifiers.h"

#include <gtkmm.h>

#include <functional>
#include <memory>

namespace Game3 {
	class ClientGame;
	class ClientInventory;
	class ItemStack;
	struct ItemSlotParent;

	class ItemSlot: public Gtk::Fixed {
		public:
			using ClickFn = std::function<void(Modifiers, int, double, double)>;

			std::function<bool(const ItemStackPtr &)> onDrop;

			ItemSlot() = delete;
			ItemSlot(const std::shared_ptr<ClientGame> &, Slot, std::shared_ptr<ClientInventory>, ItemSlotParent * = nullptr);

			~ItemSlot() override;

			void reset();
			bool empty() const;
			void setLeftClick(ClickFn);
			void setGmenu(Glib::RefPtr<Gio::Menu>);
			void setInventory(std::shared_ptr<ClientInventory>);

			void setStack(const ItemStackPtr &);
			inline auto & getStack() { return storedStack; }
			inline const auto & getStack() const { return storedStack; }

		private:
			std::weak_ptr<ClientGame> weakGame;
			Slot slot;
			std::shared_ptr<ClientInventory> inventory;
			bool durabilityVisible = false;
			ItemSlotParent *parent = nullptr;
			ItemStackPtr storedStack;

			Gtk::Image image;
			Gtk::Label label;
			Gtk::ProgressBar durabilityBar;
			Gtk::PopoverMenu popoverMenu;
			Glib::RefPtr<Gio::Menu> gmenu;

			ClickFn leftClick;

			Glib::RefPtr<Gtk::DragSource> dragSource;
			Glib::RefPtr<Gtk::GestureClick> leftGesture;
			Glib::RefPtr<Gtk::GestureClick> rightGesture;

			void addDurabilityBar(double fraction);
	};
}
