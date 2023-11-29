#pragma once

#include "data/Identifier.h"
#include "net/Buffer.h"

#include <any>
#include <memory>
#include <optional>

#include <gtkmm.h>

namespace Game3 {
	class Agent;
	class Buffer;
	class ClientGame;
	class ClientInventory;
	class InventoryModule;
	class Inventory;
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
			virtual std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &) { return {}; }
			virtual void setInventory(std::shared_ptr<ClientInventory>) {}
			virtual std::shared_ptr<InventoryModule> getPrimaryInventoryModule() { return nullptr; }
			/** Returns false if the default shift click behavior should occur, or true otherwise. */
			virtual bool handleShiftClick(std::shared_ptr<Inventory> /* source_inventory */, Slot) { return false; }
	};
}
