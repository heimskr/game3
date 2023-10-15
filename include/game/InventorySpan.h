#pragma once

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

			std::unique_ptr<Inventory> copy() const override;
			bool empty() const override;
	};
}
