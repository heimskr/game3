#pragma once

#include "item/Tool.h"

namespace Game3 {
	struct Hammer: Tool {
		Hammer(ItemID id_, const std::string &name_, MoneyCount base_price, float base_cooldown, Durability max_durability):
			Tool(id_, name_, base_price, base_cooldown, max_durability, "base:attribute/hammer"_id) {}

		bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;
		bool canUseOnWorld() const override { return true; }
	};
}
