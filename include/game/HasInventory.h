#pragma once

#include <memory>

namespace Game3 {
	class Buffer;
	class Game;
	class Inventory;
	struct Agent;

	struct HasInventory {
		HasInventory(const std::shared_ptr<Inventory> &inventory_ = nullptr):
			inventory(inventory_) {}

		std::shared_ptr<Inventory> inventory;

		void encode(Buffer &);
		void decode(Buffer &);

		protected:
			virtual std::shared_ptr<Agent> getSharedAgent() = 0;
	};
}
