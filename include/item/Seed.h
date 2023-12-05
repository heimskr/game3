#pragma once

#include "item/Plantable.h"

namespace Game3 {
	class Seed: public Plantable {
		public:
			Identifier cropTilename;

			Seed(ItemID id_, std::string name_, Identifier crop_tilename, MoneyCount base_price, ItemCount max_count = 64);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
			bool plant(InventoryPtr, Slot, ItemStack &, const Place &) override;
	};
}
