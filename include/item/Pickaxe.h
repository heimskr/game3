#pragma once

#include "item/Tool.h"

namespace Game3 {
	class Pickaxe: public Tool {
		public:
			Pickaxe(ItemID id_, std::string name_, MoneyCount base_price, double base_cooldown, Durability max_durability);

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<double, double>) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;

		private:
			static Identifier findDirtTilename(const Place &);
	};
}
