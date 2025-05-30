#pragma once

#include "item/Item.h"

namespace Game3 {
	class Floor: public Item {
		public:
			Identifier tilename;
			Floor(ItemID, std::string name, Identifier tilename, MoneyCount basePrice, ItemCount maxCount = 64);
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;
	};
}
