#pragma once

#include <memory>

namespace Game3 {
	class Agent;
	class Buffer;
	class Game;
	class Inventory;

	struct HasInventory {
		HasInventory(const std::shared_ptr<Inventory> &inventory_ = nullptr):
			inventory(inventory_) {}

		std::shared_ptr<Inventory> inventory;

		void encode(Buffer &);
		void decode(Buffer &);

		virtual void inventoryUpdated() {}

		protected:
			virtual std::shared_ptr<Agent> getSharedAgent() = 0;
	};
}
