#pragma once

#include "types/Types.h"
#include "ui/Modifiers.h"

namespace Game3 {
	struct ItemSlotParent {
		virtual ~ItemSlotParent() = default;

		virtual void slotClicked(Slot, bool is_right_click, Modifiers) = 0;
	};
}
