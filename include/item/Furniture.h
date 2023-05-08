#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Furniture: Item {
		using Item::Item;
		bool use(Slot, ItemStack &, const Place &, Modifiers) override;
	};
}
