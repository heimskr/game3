#pragma once

#include "types/Types.h"

namespace Game3 {
	struct ItemSlotParent {
		virtual ~ItemSlotParent() = default;

		virtual void slotClicked(Slot, bool is_right_click) = 0;
	};
}
