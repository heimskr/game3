#pragma once

#include "item/Tool.h"

namespace Game3 {
	class Pickaxe: public Tool {
		public:
			using Tool::Tool;

			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, ItemStack &, const Place &, Modifiers) override;
	};
}
