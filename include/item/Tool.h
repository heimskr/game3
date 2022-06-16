#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Tool: Item {
		Durability maxDurability;
		float baseCooldown;

		Tool(ItemID id_, const std::string &name_, MoneyCount base_price, float base_cooldown, ItemAttribute attribute):
		Item(id_, name_, base_price, 1), maxDurability(Item::durabilities.at(id_)), baseCooldown(base_cooldown) {
			attributes.insert(attribute);
		}
	};
}
