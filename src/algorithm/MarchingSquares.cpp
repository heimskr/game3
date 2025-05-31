#include "algorithm/MarchingSquares.h"

#include <array>
#include <unordered_map>
#include <unordered_set>

namespace Game3 {
	TileID march8(const std::function<bool(int8_t, int8_t)> &get) {
		const bool center = get(0, 0);
		int sum = 0;
		if (center) {
			uint8_t     top_left = get(-1, -1)? 1 : 0;
			uint8_t          top = get(-1,  0)? 1 : 0;
			uint8_t    top_right = get(-1,  1)? 1 : 0;
			uint8_t         left = get( 0, -1)? 1 : 0;
			uint8_t        right = get( 0,  1)? 1 : 0;
			uint8_t  bottom_left = get( 1, -1)? 1 : 0;
			uint8_t       bottom = get( 1,  0)? 1 : 0;
			uint8_t bottom_right = get( 1,  1)? 1 : 0;
			if (top == 0 || left == 0) {
				top_left = 0;
			}
			if (top == 0 || right == 0) {
				top_right = 0;
			}
			if (bottom == 0 || right == 0) {
				bottom_right = 0;
			}
			if (bottom == 0 || left == 0) {
				bottom_left = 0;
			}
			sum = top_left + (top << 1) + (top_right << 2) + (left << 3) + (right << 4) + (bottom_left << 5) + (bottom << 6) + (bottom_right << 7);
		}

		return marchingArray8[sum];
	}

	TileID march4(const std::function<bool(int8_t, int8_t)> &get) {
		const uint8_t top    = get(-1,  0)? 1 : 0;
		const uint8_t left   = get( 0, -1)? 2 : 0;
		const uint8_t right  = get( 0,  1)? 4 : 0;
		const uint8_t bottom = get( 1,  0)? 8 : 0;
		return top | left | right | bottom;
	}
}
