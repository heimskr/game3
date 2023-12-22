#pragma once

#include "SlotRange.h"
#include "game/InventoryWrapper.h"

namespace Game3 {
	class InventorySpan: public InventoryWrapper {
		private:
			SlotRange range;

		protected:
			bool validateSlot(Slot) const override;
			Slot adjustSlot(Slot) const override;

		public:
			InventorySpan(std::shared_ptr<Inventory>, SlotRange);
			InventorySpan(std::shared_ptr<Inventory>, Slot min, Slot max);

			Slot getSlotCount() const override;
			void setSlotCount(Slot) override;
			std::unique_ptr<Inventory> copy() const override;
			bool empty() const override;
			void replace(const Inventory &) override;
			void replace(Inventory &&) override;

			SlotRange getRange() const { return range; }
	};
}
