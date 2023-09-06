#pragma once

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

			std::shared_ptr<Inventory> inventory;

			void encode(Buffer &);
			void decode(Buffer &);

			virtual void inventoryUpdated() {}
			virtual std::shared_ptr<Agent> getSharedAgent() = 0;
	};
}
