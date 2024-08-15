#pragma once

#include "item/Item.h"

namespace Game3 {
	class ProjectileItem: public Item {
		public:
			ProjectileItem(ItemID id, std::string name, MoneyCount base_price, ItemCount max_count = 64);
			ProjectileItem(ItemID id, std::string name, MoneyCount base_price, Identifier projectile_id, ItemCount max_count = 64);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;

		private:
			Identifier projectileID;
	};
}
