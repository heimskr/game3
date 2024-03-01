#pragma once

#include "item/Item.h"

namespace Game3 {
	struct Bomb: Item {
		using Item::Item;
		bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
