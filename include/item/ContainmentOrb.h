#pragma once

#include "game/Fluids.h"
#include "item/Item.h"

namespace Game3 {
	class ContainmentOrb: public Item {
		public:
			ContainmentOrb(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count = 64):
				Item(std::move(id_), std::move(name_), base_price, max_count) {}

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>, Hand) override;
			std::string getTooltip(const ItemStack &) override;
			Identifier getTextureIdentifier(const ItemStack &) const override;
			bool isTextureCacheable() const override { return false; }
	};
}
