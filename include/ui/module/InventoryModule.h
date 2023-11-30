#pragma once

#include "interface/ItemSlotParent.h"
#include "types/Types.h"
#include "ui/Modifiers.h"
#include "ui/gtk/ItemSlot.h"
#include "ui/module/Module.h"

#include <any>
#include <functional>
#include <memory>
#include <vector>

namespace Game3 {
	class Agent;
	class ClientInventory;
	class InventoryTab;

	class InventoryModule: public Module, public ItemSlotParent, public std::enable_shared_from_this<InventoryModule> {
		public:
			struct Argument {
				std::shared_ptr<Agent> agent;
				InventoryID index;
			};

			static Identifier ID() { return {"base", "module/inventory"}; }

			using GmenuFn = std::function<void(InventoryModule &, Glib::RefPtr<Gio::Menu>)>;

			InventoryModule(std::shared_ptr<ClientGame>, const std::any &, ItemSlotParent * = nullptr, const GmenuFn & = {});
			InventoryModule(std::shared_ptr<ClientGame>, std::shared_ptr<ClientInventory>, ItemSlotParent * = nullptr, const GmenuFn & = {});

			Identifier getID() const final { return ID(); }
			Gtk::Widget & getWidget() final;
			void reset()  final;
			void update() final;
			void onResize(int) final;
			std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) final;
			std::shared_ptr<InventoryModule> getPrimaryInventoryModule() final { return shared_from_this(); }

			bool addCSSClass(const Glib::ustring &, Slot);
			void removeCSSClass(const Glib::ustring &, Slot = -1);
			inline auto getInventory() const { return inventory; }
			void setInventory(std::shared_ptr<ClientInventory>) override;
			void slotClicked(Slot, bool is_right_click, Modifiers) override;

		private:
			std::shared_ptr<ClientGame> game;
			std::shared_ptr<ClientInventory> inventory;
			Glib::RefPtr<Gio::Menu> gmenu;
			Glib::RefPtr<Gtk::DragSource> source;
			Glib::ustring name;
			std::vector<std::unique_ptr<ItemSlot>> itemSlots;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Gtk::Box hbox{Gtk::Orientation::HORIZONTAL};
			Gtk::PopoverMenu popoverMenu;
			Gtk::FlowBox flowBox;
			Gtk::Label label;
			Slot lastSlot = -1;
			Slot lastSlotCount = -1;
			int tabWidth = 0;
			ItemSlotParent *parent = nullptr;

			static std::shared_ptr<ClientInventory> getInventory(const std::any &);
			static InventoryID getInventoryIndex(const std::any &);
			int gridWidth() const;
			void populate();
			void repopulate();
			void leftClick(Slot, Modifiers);
	};
}
