#include "algorithm/MarchingSquares.h"

#include <array>
#include <unordered_map>
#include <unordered_set>

namespace Game3 {
	std::unordered_map<int, int> marchingMap8 {
		{  0, 40}, {  2, 20}, {  8, 33}, { 10, 26}, { 11, 23}, { 16, 31}, { 18, 24}, { 22, 21}, { 24, 32}, { 26, 25}, { 27, 46}, { 30, 45}, { 31, 22}, { 64,  0}, { 66, 10}, { 72,  6}, { 74, 16},
		{ 75, 35}, { 80,  4}, { 82, 14}, { 86, 36}, { 88,  5}, { 90, 18}, { 91, 38}, { 94, 39}, { 95,  8}, {104,  3}, {106, 47}, {107, 13}, {120, 34}, {122, 48}, {123, 17}, {126, 43}, {127,  7},
		{208,  1}, {210, 44}, {214, 11}, {216, 37}, {218, 49}, {219, 42}, {222, 19}, {223,  9}, {248,  2}, {250, 28}, {251, 27}, {254, 29}, {255, 12},
	};

	TileID march8(const std::function<bool(int8_t, int8_t)> &get) {
		const bool center = get(0, 0);
		int sum = 0;
		if (center) {
			uint8_t     top_left = get(-1, -1)? 1 : 0;
			const uint8_t    top = get( 0, -1)? 1 : 0;
			uint8_t    top_right = get( 1, -1)? 1 : 0;
			const uint8_t   left = get(-1,  0)? 1 : 0;
			const uint8_t  right = get( 1,  0)? 1 : 0;
			uint8_t  bottom_left = get(-1,  1)? 1 : 0;
			const uint8_t bottom = get( 0,  1)? 1 : 0;
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

		return !center || sum != 0? marchingMap8.at(sum) : 15;
	}

	TileID march4(const std::function<bool(int8_t, int8_t)> &get) {
		const uint8_t top    = get(-1,  0)? 1 : 0;
		const uint8_t left   = get( 0, -1)? 2 : 0;
		const uint8_t right  = get( 0,  1)? 4 : 0;
		const uint8_t bottom = get( 1,  0)? 8 : 0;
		return top | left | right | bottom;
	}
}
