#pragma once

#include "game/ServerInventory.h"

namespace Game3 {
	/** Represents an inventory with no restrictions on stack sizes. */
	class ExpandedServerInventory: public ServerInventory {
		public:
			using ServerInventory::ServerInventory;

			std::unique_ptr<Inventory> copy() const override;
			std::optional<ItemStack> add(const ItemStack &, const std::function<bool(Slot)> &, Slot start) override;
	};
}
