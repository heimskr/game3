#pragma once

#include "game/ServerInventory.h"

namespace Game3 {
	/** Represents an inventory with no restrictions on stack sizes. */
	class ExpandedServerInventory: public ServerInventory {
		public:
			using ServerInventory::ServerInventory;

			ExpandedServerInventory(const ServerInventory &other):
			ServerInventory(other) {}

			ExpandedServerInventory(ServerInventory &&other):
				ServerInventory(std::move(other)) {}

			std::unique_ptr<Inventory> copy() const override;
			std::optional<ItemStack> add(const ItemStack &, const std::function<bool(Slot)> &, Slot start) override;

			bool canInsert(const ItemStack &, const std::function<bool(Slot)> &) const override;
			bool canInsert(const ItemStack &, Slot) const override;
	};

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ExpandedServerInventory popBuffer<ExpandedServerInventory>(Buffer &);
	Buffer & operator+=(Buffer &, const ExpandedServerInventory &);
	Buffer & operator<<(Buffer &, const ExpandedServerInventory &);
	Buffer & operator>>(Buffer &, ExpandedServerInventory &);
}
