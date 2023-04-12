#pragma once

#include "item/Item.h"
#include "item/HasMaxDurability.h"

namespace Game3 {
	struct Tool: Item, HasMaxDurability {
		float baseCooldown;

		Tool(ItemID id_, const std::string &name_, MoneyCount base_price, float base_cooldown, Durability max_durability, ItemAttribute attribute):
		Item(id_, name_, base_price, 1), HasMaxDurability(max_durability), baseCooldown(base_cooldown) {
			attributes.insert(attribute);
		}

		void initStack(const Game &, ItemStack &) override;
	};
}
