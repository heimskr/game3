#pragma once

#include "item/Item.h"
#include "item/HasMaxDurability.h"

namespace Game3 {
	struct Tool: Item, HasMaxDurability {
		float baseCooldown;

		Tool(ItemID id_, std::string name_, MoneyCount base_price, float base_cooldown, Durability max_durability, Identifier attribute):
		Item(id_, std::move(name_), base_price, 1), HasMaxDurability(max_durability), baseCooldown(base_cooldown) {
			attributes.insert(std::move(attribute));
		}

		void initStack(const Game &, ItemStack &) override;
	};
}
