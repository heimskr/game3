#pragma once

#include <array>
#include <functional>
#include <unordered_map>

#include "Position.h"
#include "Types.h"

namespace Game3 {
	extern std::unordered_map<int, int> marchingMap8;
	extern std::array<int, 16> marchingArray4;

	TileID march8(const Position &, const std::function<bool(int8_t, int8_t)> &);
	TileID march4(const Position &, const std::function<bool(int8_t, int8_t)> &);
}
