#pragma once

#include "item/Item.h"

namespace Game3 {
	class TerrainSeed: public Item {
		public:
			Identifier targetTilename;
			Identifier replacementTilename;

			TerrainSeed(ItemID id_, std::string name_, Identifier target_tilename, Identifier replacement_tilename, MoneyCount base_price, ItemCount max_count = 64);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
	};
}
