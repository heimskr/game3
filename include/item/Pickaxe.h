#pragma once

#include "item/Tool.h"

namespace Game3 {
	class Pickaxe: public Tool {
		public:
			Pickaxe(ItemID id_, std::string name_, MoneyCount base_price, float base_cooldown, Durability max_durability);

			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;

		private:
			static Identifier findDirtTilename(const Place &);
	};
}
