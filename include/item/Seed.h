#pragma once

#include "item/Item.h"

namespace Game3 {
	class Seed: public Item {
		public:
			Identifier cropTilename;

			Seed(ItemID id_, std::string name_, Identifier crop_tilename, MoneyCount base_price, ItemCount max_count = 64);

			bool use(Slot, ItemStack &, const Place &, Modifiers) override;
	};
}
