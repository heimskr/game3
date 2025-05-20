#pragma once

#include "item/Tool.h"

namespace Game3 {
	class VoidPickaxe: public Tool {
		public:
			using Tool::Tool;
			bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>) override;
			bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float>, DragAction) override;
			bool canUseOnWorld() const override { return false; }
	};
}
