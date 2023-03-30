#pragma once

#include "item/Tool.h"

namespace Game3 {
	struct Hammer: Tool {
		Hammer(ItemID id_, const std::string &name_, MoneyCount base_price, float base_cooldown):
			Tool(id_, name_, base_price, base_cooldown, ItemAttribute::Hammer) {}

		bool use(Slot, ItemStack &, const Place &) override;
	};
}
