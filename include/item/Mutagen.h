#pragma once

#include "item/Item.h"

namespace Game3 {
	class Mutagen: public Item {
		public:
			using Item::Item;
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}
