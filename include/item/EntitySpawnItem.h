#pragma once

#include "item/Item.h"

namespace Game3 {
	class EntitySpawnItem: public Item {
		public:
			Identifier entityID;

			EntitySpawnItem(ItemID id_, std::string name_, MoneyCount base_price, Identifier entity_id, ItemCount max_count = 64);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
	};
}

