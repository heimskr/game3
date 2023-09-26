#pragma once

#include <array>
#include <functional>
#include <unordered_map>

#include "Position.h"
#include "Types.h"

namespace Game3 {
	extern std::unordered_map<int, int> marchingMap8;
	constexpr std::array<int, 16> marchingArray4 {22 /* TODO: verify */, 17, 6, 16, 4, 14, 1, 26, 3, 7, 2, 20, 0, 18, 12, 19};
	// Too lazy to rearrange the pipe sets manually and I don't feel like bothering Luna with more work.
	constexpr std::array<int, 16> marchingArray4Alt {0, 1, 4, 5, 8, 9, 12, 13, 2, 3, 6, 7, 10, 11, 14, 15};

	TileID march8(const std::function<bool(int8_t, int8_t)> &);
	TileID march4(const std::function<bool(int8_t, int8_t)> &);
}
