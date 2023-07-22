#pragma once

#include <unordered_map>
#include <utility>

#include "ui/tab/Tab.h"

namespace Game3 {
	class ClientGame;
	class ClientInventory;
	class MainWindow;
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
			void setExternalInventory(const Glib::ustring &name, const std::shared_ptr<ClientInventory> &, const std::shared_ptr<Agent> &);
			void resetExternalInventory();
			std::shared_ptr<ClientInventory> getExternalInventory() const { return externalInventory; }
			GlobalID getExternalGID() const;

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::Grid playerGrid;
			Gtk::Grid externalGrid;
			Gtk::Label externalLabel;
			Gtk::Box vbox {Gtk::Orientation::VERTICAL};
			Gtk::Box hbox {Gtk::Orientation::HORIZONTAL};
			Gtk::PopoverMenu popoverMenu;
			std::vector<std::unique_ptr<Gtk::Widget>> playerWidgets;
			std::vector<std::unique_ptr<Gtk::Widget>> externalWidgets;
			int lastGridWidth = 0;
			std::shared_ptr<ClientInventory> externalInventory;
			std::weak_ptr<Agent> externalAgent;
			Glib::ustring externalName;
			std::unordered_map<Gtk::Widget *, std::pair<Slot, bool>> widgetMap;
			std::unordered_map<Slot, Gtk::Widget *> playerWidgetsBySlot;
			std::unordered_map<Slot, Gtk::Widget *> externalWidgetsBySlot;
			Glib::RefPtr<Gio::Menu> gmenuSelf;
			Glib::RefPtr<Gio::Menu> gmenuExternal;

			/** We can't store state in a popover, so we have to store it here. */
			std::shared_ptr<ClientGame> lastGame;
			Slot lastSlot = -1;
			bool lastExternal = false;

			Slot draggedSlot = -1;
			bool draggedExternal = false;

			int gridWidth() const;
			void leftClick(const std::shared_ptr<ClientGame> &, Gtk::Widget *, int click_count, Slot, bool external, double x, double y);
			void rightClick(const std::shared_ptr<ClientGame> &, Gtk::Widget *, int click_count, Slot, bool external, double x, double y);
			void updatePlayerClasses(const std::shared_ptr<ClientGame> &);
			void populate(Gtk::Grid &, ClientInventory &, bool external);
	};
}
