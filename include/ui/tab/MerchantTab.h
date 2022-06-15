#pragma once

#include "Types.h"
#include "ui/tab/Tab.h"

namespace Game3 {
	class Inventory;
	class MainWindow;

	class MerchantTab: public Tab {
		public:
			MainWindow &mainWindow;

			MerchantTab() = delete;
			MerchantTab(MainWindow &);

			MerchantTab(const MerchantTab &) = delete;
			MerchantTab(MerchantTab &&) = delete;

			MerchantTab & operator=(const MerchantTab &) = delete;
			MerchantTab & operator=(MerchantTab &&) = delete;

			Gtk::Widget & getWidget() override { return scrolled; }
			Glib::ustring getName() override { return merchantName.empty()? "Merchant" : merchantName; }
			void onResize(const std::shared_ptr<Game> &) override;
			void update(const std::shared_ptr<Game> &) override;
			void reset(const std::shared_ptr<Game> &) override;
			void setMerchantInventory(const Glib::ustring &name, const std::shared_ptr<Inventory> &, double price_multiplier);
			void resetMerchantInventory();
			std::shared_ptr<Inventory> getMerchantInventory() const { return merchantInventory; }

		private:
			constexpr static int TILE_MARGIN = 2;
			constexpr static int TILE_SIZE = 64;
			constexpr static int TILE_MAGIC = 5;

			Gtk::ScrolledWindow scrolled;
			Gtk::Grid playerGrid;
			Gtk::Grid merchantGrid;
			Gtk::Label merchantLabel;
			Gtk::Box vbox {Gtk::Orientation::VERTICAL};
			Gtk::Box hbox {Gtk::Orientation::HORIZONTAL};
			Gtk::PopoverMenu popoverMenu;
			std::vector<std::unique_ptr<Gtk::Widget>> playerWidgets;
			std::vector<std::unique_ptr<Gtk::Widget>> merchantWidgets;
			int lastGridWidth = 0;
			std::shared_ptr<Inventory> merchantInventory;
			Glib::ustring merchantName;

			/** We can't store state in a popover, so we have to store it here. */
			std::shared_ptr<Game> lastGame;
			Slot lastSlot = -1;
			bool lastMerchant = false;
			double priceMultiplier = 1.;

			int gridWidth() const;
			void leftClick(const std::shared_ptr<Game> &, Gtk::Widget *, int click_count, Slot, bool merchant, double x, double y);
			void rightClick(const std::shared_ptr<Game> &, Gtk::Widget *, int click_count, Slot, bool merchant, double x, double y);
			Gtk::Widget & getDraggedItem();
	};
}
