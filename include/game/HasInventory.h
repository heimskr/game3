#pragma once

#include "types/Types.h"

#include <memory>

namespace Game3 {
	class Agent;
	class Buffer;
	class Game;
	class Inventory;

	class HasInventory {
		public:
			HasInventory(std::shared_ptr<Inventory> inventory_ = nullptr):
				inventory(std::move(inventory_)) {}

			virtual ~HasInventory() = default;

			virtual const std::shared_ptr<Inventory> & getInventory(InventoryID) const;
			virtual void setInventory(std::shared_ptr<Inventory>, InventoryID);
			virtual InventoryID getInventoryCount() const { return 1; }

			void encode(Buffer &, InventoryID);
			void decode(Buffer &, InventoryID);

			virtual void inventoryUpdated() {}
			virtual std::shared_ptr<Agent> getSharedAgent() = 0;

		private:
			std::shared_ptr<Inventory> inventory;
	};
}
