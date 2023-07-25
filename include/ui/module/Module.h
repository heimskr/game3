#pragma once

#include <gtkmm.h>

namespace Game3 {
	class ClientGame;
	class InventoryTab;

	/** Displayed below the inventory in the inventory tab. */
	class Module {
		protected:
			Module() = default;

		public:
			virtual ~Module() = default;

			virtual Gtk::Widget & getWidget() = 0;
			virtual void reset()  = 0;
			virtual void update() = 0;
			virtual void onResize(int /* width */) {}
	};
}
