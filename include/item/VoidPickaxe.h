#pragma once

#include "item/Tool.h"

namespace Game3 {
	class VoidPickaxe: public Tool {
		public:
			using Tool::Tool;
			bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float>) override;
			bool canUseOnWorld() const override { return true; }
	};
}
