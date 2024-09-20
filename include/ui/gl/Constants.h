#pragma once

#include "graphics/Color.h"
#include "types/Types.h"

#include <array>
#include <string_view>

namespace Game3 {
	constexpr double X_FRACTION = 0.2;
	constexpr double Y_FRACTION = 0.2;
	constexpr double UI_SCALE = 8;
	constexpr double SLOT_SCALE = 8;
	constexpr double UNSCALE = 1.6;
	constexpr float TEXT_INPUT_HEIGHT_FACTOR = 11;
	constexpr int TOP_OFFSET = 20 * UI_SCALE;
	constexpr int UNSCALED = 6 * UI_SCALE / UNSCALE;
	constexpr int INNER_SLOT_SIZE = 16;
	constexpr int OUTER_SLOT_SIZE = INNER_SLOT_SIZE * 5 / 4;
	constexpr int SLOT_PADDING = OUTER_SLOT_SIZE - INNER_SLOT_SIZE;
	constexpr Slot HOTBAR_SIZE = 10;
	constexpr float HOTBAR_SCALE = 6;
	constexpr float HOTBAR_BORDER = SLOT_PADDING * HOTBAR_SCALE / 3;
	constexpr Color DEFAULT_TEXTINPUT_INTERIOR_COLOR{"#fdeed3"};
	constexpr Color DEFAULT_BACKGROUND_COLOR{0.88, 0.77, 0.55};
	constexpr std::array<std::string_view, 8> FRAME_PIECES{
		"resources/gui/gui_topleft.png", "resources/gui/gui_top.png", "resources/gui/gui_topright.png", "resources/gui/gui_right.png",
		"resources/gui/gui_bottomright.png", "resources/gui/gui_bottom.png", "resources/gui/gui_bottomleft.png", "resources/gui/gui_left.png",
	};
}
