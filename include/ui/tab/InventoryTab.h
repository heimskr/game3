#include "ui/tab/Tab.h"

namespace Game3 {
	class MainWindow;

	class InventoryTab: public Tab {
		public:
			MainWindow &mainWindow;

			InventoryTab() = delete;
			InventoryTab(const InventoryTab &) = delete;
			InventoryTab(InventoryTab &&) = delete;
			InventoryTab(MainWindow &);

			InventoryTab & operator=(const InventoryTab &) = delete;
			InventoryTab & operator=(InventoryTab &&) = delete;

			Gtk::Widget & getWidget() override { return scrolled; }
			Glib::ustring getName() override { return "Inventory"; }
			void onFocus() override;
			void onBlur() override;

			void update(const std::shared_ptr<Game> &) override;
			void reset(const std::shared_ptr<Game> &) override;

		private:
			constexpr static int TILE_MARGIN = 2;
			constexpr static int TILE_SIZE = 100 - 2 * TILE_MARGIN;

			Gtk::Grid grid;
			Gtk::ScrolledWindow scrolled;
			Gtk::PopoverMenu popoverMenu;
			std::vector<std::unique_ptr<Gtk::Widget>> gridWidgets;

			/** We can't store state in a popover, so we have to store it here. */
			std::shared_ptr<Game> lastGame;
			Slot lastSlot = -1;

			void leftClick(const std::shared_ptr<Game> &, Gtk::Label &, int click_count, Slot, double x, double y);
			void rightClick(const std::shared_ptr<Game> &, Gtk::Label &, int click_count, Slot, double x, double y);
	};
}
