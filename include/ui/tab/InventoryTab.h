#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "interface/ItemSlotParent.h"
#include "types/Types.h"
#include "threading/LockableSharedPtr.h"
#include "ui/Modifiers.h"
#include "ui/module/InventoryModule.h"
#include "ui/tab/Tab.h"

namespace Game3 {
	class Agent;
	class ClientGame;
	class ClientInventory;
	class MainWindow;
	class Module;
	class TileEntity;

	class InventoryTab: public Tab, public ItemSlotParent {
		public:
			constexpr static int TILE_MARGIN = 2;
			constexpr static int TILE_SIZE = 64;
			constexpr static int TILE_MAGIC = 5;

			MainWindow &mainWindow;

			InventoryTab() = delete;
			InventoryTab(MainWindow &);

			InventoryTab(const InventoryTab &) = delete;
			InventoryTab(InventoryTab &&) = delete;

			InventoryTab & operator=(const InventoryTab &) = delete;
			InventoryTab & operator=(InventoryTab &&) = delete;

			Gtk::Widget & getWidget() override { return scrolled; }
			std::string getName() override { return "Inventory"; }
			void onResize(const std::shared_ptr<ClientGame> &) override;
			void update(const std::shared_ptr<ClientGame> &) override;
			void reset(const std::shared_ptr<ClientGame> &) override;
			void setModule(std::shared_ptr<Module>);
			Module & getModule() const;
			Module * getModule(std::shared_lock<DefaultMutex> &);
			Module * getModule(std::unique_lock<DefaultMutex> &);
			void removeModule();
			GlobalID getExternalGID() const;

			void slotClicked(Slot, bool is_right_click, Modifiers) override;
			void slotDoubleClicked(Slot) override;
			bool suppressLeftClick() const override { return true; }
			void activeSlotSet();

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Gtk::PopoverMenu popoverMenu;
			Lockable<std::shared_ptr<Module>> currentModule;
			std::optional<InventoryModule> inventoryModule;
			Gtk::Box actionBox{Gtk::Orientation::HORIZONTAL};
			Gtk::Image holdLeftAction;
			Gtk::Image holdRightAction;
			Gtk::Image dropAction;
			Gtk::Image discardAction;

			/** We can't store state in a popover, so we have to store it here. */
			LockableSharedPtr<ClientGame> lastGame;
			Slot lastSlot = -1;

			int lastWidth = -1;

			int gridWidth() const;
			void leftClick(Slot, Modifiers);
			void shiftClick(const std::shared_ptr<ClientGame> &, Slot);
			void updatePlayerClasses(const std::shared_ptr<ClientGame> &);
			void populate(std::shared_ptr<ClientInventory>);
			void clear();
			void gmenuSetup(InventoryModule &, Glib::RefPtr<Gio::Menu>);
			void updateInventory(const std::shared_ptr<ClientGame> &);
			void initAction(Gtk::Image &, const Glib::ustring &icon, const Glib::ustring &tooltip, std::function<void(Slot)>);
	};
}
