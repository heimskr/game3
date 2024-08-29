#pragma once

#include "types/Types.h"

#include <memory>

namespace Game3 {
	class Inventory;

	class InventoryGetter {
		public:
			InventoryGetter(const Inventory &);

			InventoryGetter(const InventoryGetter &) = delete;
			InventoryGetter(InventoryGetter &&) = delete;

			virtual ~InventoryGetter() = default;

			InventoryGetter & operator=(const InventoryGetter &) = delete;
			InventoryGetter & operator=(InventoryGetter &&) = delete;

			virtual std::shared_ptr<Inventory> get() const;
			operator std::shared_ptr<Inventory>() const;

		private:
			std::shared_ptr<Agent> owner;
			InventoryID index;
	};
}
