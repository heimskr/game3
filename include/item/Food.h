#pragma once

#include "item/Item.h"

namespace Game3 {
	class Food: public Item {
		public:
			using Item::Item;

			bool use(Slot, const ItemStackPtr &, const std::shared_ptr<Player> &, Modifiers) override;

			virtual HitPoints getHealedPoints(const std::shared_ptr<Player> &) = 0;
			/** Returns whether the food should be consumed even if it doesn't end up restoring any health. */
			virtual bool getConsumptionForced() { return false; }
	};
}
