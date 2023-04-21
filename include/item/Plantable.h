#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Plantable: Item {
		Identifier tilename;
		/** Can be empty to indicate that the plantable can be planted on any walkable tile. */
		Identifier validGround;

		Plantable(ItemID, std::string name_, Identifier tilename_, Identifier valid_ground, MoneyCount base_price, ItemCount max_count = 64);

		bool use(Slot, ItemStack &, const Place &) override;
	};
}
