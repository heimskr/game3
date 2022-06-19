#include "MarchingSquares.h"

namespace Game3 {
	std::unordered_map<int, int> marchingMap8 {
		{  0, 40}, {  2, 20}, {  8, 33}, { 10, 26}, { 11, 23}, { 16, 31}, { 18, 24}, { 22, 21}, { 24, 32}, { 26, 25}, { 27, 46}, { 30, 45}, { 31, 22}, { 64,  0}, { 66, 10}, { 72,  6}, { 74, 16},
		{ 75, 35}, { 80,  4}, { 82, 14}, { 86, 36}, { 88,  5}, { 90, 18}, { 91, 38}, { 94, 39}, { 95,  8}, {104,  3}, {106, 47}, {107, 13}, {120, 34}, {122, 48}, {123, 17}, {126, 43}, {127,  7},
		{208,  1}, {210, 44}, {214, 11}, {216, 37}, {218, 49}, {219, 42}, {222, 19}, {223,  9}, {248,  2}, {250, 28}, {251, 27}, {254, 29}, {255, 12},
	};

	std::array<int, 16> marchingArray4 {22 /* TODO: verify */, 17, 6, 16, 4, 14, 1, 26, 3, 10, 2, 20, 0, 18, 12, 19};

	TileID march8(const Position &position, const std::function<bool(int8_t, int8_t)> &get) {
		const bool center = get(0, 0);
		int sum = 0;
		if (center) {
			auto     top_left = static_cast<uint8_t>(get(position.column - 1, position.row - 1));
			const auto    top = static_cast<uint8_t>(get(position.column,     position.row - 1));
			auto    top_right = static_cast<uint8_t>(get(position.column + 1, position.row - 1));
			const auto   left = static_cast<uint8_t>(get(position.column - 1, position.row    ));
			const auto  right = static_cast<uint8_t>(get(position.column + 1, position.row    ));
			auto  bottom_left = static_cast<uint8_t>(get(position.column - 1, position.row + 1));
			const auto bottom = static_cast<uint8_t>(get(position.column,     position.row + 1));
			auto bottom_right = static_cast<uint8_t>(get(position.column + 1, position.row + 1));
			if (!top || !left)
				top_left = 0;
			if (!top || !right)
				top_right = 0;
			if (!bottom || !right)
				bottom_right = 0;
			if (!bottom || !left)
				bottom_left = 0;
			sum = top_left + (top << 1) + (top_right << 2) + (left << 3) + (right << 4) + (bottom_left << 5) + (bottom << 6) + (bottom_right << 7);
		}

		return !center || sum != 0? marchingMap8.at(sum) : 15;
	}

	TileID march4(const Position &position, const std::function<bool(int8_t, int8_t)> &get) {
		const bool center = get(0, 0);
		int sum = 0;
		if (center) {
			const auto top    = static_cast<uint8_t>(get(position.column,     position.row - 1));
			const auto left   = static_cast<uint8_t>(get(position.column - 1, position.row    ));
			const auto right  = static_cast<uint8_t>(get(position.column + 1, position.row    ));
			const auto bottom = static_cast<uint8_t>(get(position.column,     position.row + 1));
			sum = top + (left << 1) + (right << 2) + (bottom << 3);
		}

		return !center || sum != 0? marchingArray4.at(sum) : 22;
	}
}
