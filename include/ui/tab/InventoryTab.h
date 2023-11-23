#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "types/Types.h"
#include "threading/LockableSharedPtr.h"
#include "ui/Modifiers.h"
#include "ui/tab/Tab.h"

namespace Game3 {
	class Agent;
	class ClientGame;
	class ClientInventory;
	class MainWindow;
	class Module;
	class TileEntity;

	class InventoryTab: public Tab {
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

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::Grid grid;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Gtk::PopoverMenu popoverMenu;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;
			int lastGridWidth = 0;
			std::unordered_map<Gtk::Widget *, Slot> widgetMap;
			std::unordered_map<Slot, Gtk::Widget *> widgetsBySlot;
			Glib::RefPtr<Gio::Menu> gmenu;
			Lockable<std::shared_ptr<Module>> currentModule;
			std::unordered_map<Gtk::Widget *, std::pair<Glib::RefPtr<Gtk::GestureClick>, Glib::RefPtr<Gtk::GestureClick>>> clickGestures;

			/** We can't store state in a popover, so we have to store it here. */
			LockableSharedPtr<ClientGame> lastGame;
			Slot lastSlot = -1;

			int gridWidth() const;
			void leftClick(const std::shared_ptr<ClientGame> &, Gtk::Widget *, int click_count, Slot, Modifiers, double x, double y);
			void rightClick(const std::shared_ptr<ClientGame> &, Gtk::Widget *, int click_count, Slot, Modifiers, double x, double y);
			void shiftClick(const std::shared_ptr<ClientGame> &, Slot);
			void updatePlayerClasses(const std::shared_ptr<ClientGame> &);
			void populate(Gtk::Grid &, std::shared_ptr<ClientInventory>);
			void clear();
	};
}
