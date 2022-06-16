#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Tool: Item {
		Durability maxDurability;
		float baseCooldown;

		Tool(ItemID id_, const std::string &name_, MoneyCount base_price, Durability max_durability, float base_cooldown, ItemAttribute attribute):
		Item(id_, name_, base_price, 1), maxDurability(max_durability), baseCooldown(base_cooldown) {
			addAttribute(attribute);
		}
	};
}
