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

			virtual ~HasInventory() = default;

			inline const auto & getInventory() const { return inventory; }
			void setInventory(std::shared_ptr<Inventory>);

			void encode(Buffer &);
			void decode(Buffer &);

			virtual void inventoryUpdated() {}
			virtual std::shared_ptr<Agent> getSharedAgent() = 0;

		private:
			std::shared_ptr<Inventory> inventory;
	};
}
