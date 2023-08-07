#pragma once

#include "data/Identifier.h"

#include <gtkmm.h>

namespace Game3 {
	class Agent;
	class ClientGame;
	class InventoryTab;

	/** Displayed below the inventory in the inventory tab. */
	class Module {
		protected:
			Module() = default;

		public:
			virtual ~Module() = default;

			virtual Identifier getID() const = 0;
			virtual Gtk::Widget & getWidget() = 0;
			virtual void reset()  = 0;
			virtual void update() = 0;
			virtual void onResize(int /* width */) {}
			virtual void handleMessage(Agent &, const std::string &, Buffer &) {}
	};
}
