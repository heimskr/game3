#pragma once

#include "types/Types.h"

namespace Game3 {
	struct SlotRange {
		Slot min;
		Slot max;

		SlotRange(Slot min_, Slot max_):
			min(min_), max(max_) {}

		inline bool contains(Slot slot) const {
			return min <= slot && slot <= max;
		}

		inline Slot size() const {
			return max - min + 1;
		}
	};
}
