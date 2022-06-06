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
			constexpr static int TILE_SIZE = 80;

			Gtk::Grid grid;
			Gtk::ScrolledWindow scrolled;
			Gtk::PopoverMenu popoverMenu;
			std::vector<std::unique_ptr<Gtk::Widget>> gridWidgets;

			void leftClick(const std::shared_ptr<Game> &, Gtk::Label &, int click_count, Slot, double x, double y);
			void rightClick(const std::shared_ptr<Game> &, Gtk::Label &, int click_count, Slot, double x, double y);
	};
}
