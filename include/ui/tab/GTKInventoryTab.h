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
#include "ui/module/GTKInventoryModule.h"
#include "ui/tab/GTKTab.h"

namespace Game3 {
	class Agent;
	class ClientGame;
	class ClientInventory;
	class GTKModule;
	class MainWindow;
	class TileEntity;

	class GTKInventoryTab: public GTKTab, public ItemSlotParent {
		public:
			constexpr static int TILE_MARGIN = 2;
			constexpr static int TILE_SIZE = 64;
			constexpr static int TILE_MAGIC = 5;

			MainWindow &mainWindow;

			GTKInventoryTab() = delete;
			GTKInventoryTab(MainWindow &);

			~GTKInventoryTab() override;

			Gtk::Widget & getWidget() override { return scrolled; }
			std::string getName() override { return "Modules"; }
			void onResize(const std::shared_ptr<ClientGame> &) override;
			void update(const std::shared_ptr<ClientGame> &) override;
			void reset(const std::shared_ptr<ClientGame> &) override;
			void setModule(std::shared_ptr<GTKModule>);
			GTKModule & getModule() const;
			GTKModule * getModule(std::shared_lock<DefaultMutex> &);
			GTKModule * getModule(std::unique_lock<DefaultMutex> &);
			void removeModule();
			GlobalID getExternalGID() const;

			void slotClicked(Slot, bool is_right_click, Modifiers) override;
			void slotDoubleClicked(Slot) override;
			bool suppressLeftClick() const override { return true; }
			void activeSlotSet();

		private:
			Gtk::ScrolledWindow scrolled;
			Gtk::Box vbox{Gtk::Orientation::VERTICAL};
			Lockable<std::shared_ptr<GTKModule>> currentModule;
			std::optional<GTKInventoryModule> inventoryModule;
			Gtk::Box actionBox{Gtk::Orientation::HORIZONTAL};
			Gtk::Image holdLeftAction;
			Gtk::Image holdRightAction;
			Gtk::Image dropAction;
			Gtk::Image discardAction;
			Glib::RefPtr<Gio::SimpleActionGroup> group;
			Glib::RefPtr<Gtk::EventControllerMotion> motion;

			/** We can't store state in a popover, so we have to store it here. */
			std::shared_ptr<ClientGame> lastGame;
			Slot lastSlot = -1;
			int lastWidth = -1;
			Modifiers lastModifiers;

			int gridWidth() const;
			void leftClick(Slot, Modifiers);
			void shiftClick(const std::shared_ptr<ClientGame> &, Slot);
			void updatePlayerClasses(const std::shared_ptr<ClientGame> &);
			void populate(std::shared_ptr<ClientInventory>);
			void clear();
			void gmenuSetup(GTKInventoryModule &, Glib::RefPtr<Gio::Menu>, Slot, const ItemStackPtr &);
			void updateInventory(const std::shared_ptr<ClientGame> &);
			void initAction(Gtk::Image &, const Glib::ustring &icon, const Glib::ustring &tooltip, std::function<void(Slot, Modifiers)>);
	};
}
