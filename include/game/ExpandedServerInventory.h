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

			using StorageInventory::operator=;

			std::unique_ptr<Inventory> copy() const override;
			ItemStackPtr add(const ItemStackPtr &, const SlotPredicate &, Slot start) override;

			bool canInsert(const ItemStackPtr &, const SlotPredicate &) const override;
			bool canInsert(const ItemStackPtr &, Slot) const override;
	};

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ExpandedServerInventory popBuffer<ExpandedServerInventory>(Buffer &);
	Buffer & operator+=(Buffer &, const ExpandedServerInventory &);
	Buffer & operator<<(Buffer &, const ExpandedServerInventory &);
	Buffer & operator>>(Buffer &, ExpandedServerInventory &);
}
