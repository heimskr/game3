#pragma once

#include "item/Plantable.h"

namespace Game3 {
	struct Flower: Plantable {
		Identifier tilename;
		/** Can be empty to indicate that the flower can be planted on any walkable tile. */
		Identifier validGround;

		Flower(ItemID, std::string name_, Identifier tilename_, Identifier valid_ground, MoneyCount base_price, ItemCount max_count = 64);

		bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
		bool plant(InventoryPtr, Slot, ItemStack &, const Place &) override;
	};
}
