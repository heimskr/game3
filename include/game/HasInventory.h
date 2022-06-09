#pragma once

#include <memory>

namespace Game3 {
	class Inventory;

	struct HasInventory {
		HasInventory(const std::shared_ptr<Inventory> &inventory_ = nullptr):
			inventory(inventory_) {}

		std::shared_ptr<Inventory> inventory;
	};
}
