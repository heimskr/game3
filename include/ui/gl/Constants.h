#pragma once

#include "types/Types.h"

namespace Game3 {
	constexpr double X_FRACTION = 0.2;
	constexpr double Y_FRACTION = 0.2;
	constexpr double SCALE = 8;
	constexpr double SLOT_SCALE = 8;
	constexpr double UNSCALE = 1.6;
	constexpr float TEXT_INPUT_HEIGHT_FACTOR = 11;
	constexpr int TOP_OFFSET = 20 * SCALE;
	constexpr int UNSCALED = 6 * SCALE / UNSCALE;
	constexpr int INNER_SLOT_SIZE = 16;
	constexpr int OUTER_SLOT_SIZE = INNER_SLOT_SIZE * 5 / 4;
	constexpr int SLOT_PADDING = OUTER_SLOT_SIZE - INNER_SLOT_SIZE;
	constexpr Slot HOTBAR_SIZE = 10;
}
