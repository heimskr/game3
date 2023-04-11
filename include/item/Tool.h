#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Tool: Item {
		float baseCooldown;
		Durability maxDurability;

		Tool(ItemID id_, const std::string &name_, MoneyCount base_price, float base_cooldown, Durability max_durability, ItemAttribute attribute):
		Item(id_, name_, base_price, 1), baseCooldown(base_cooldown), maxDurability(max_durability) {
			attributes.insert(attribute);
		}
	};
}
