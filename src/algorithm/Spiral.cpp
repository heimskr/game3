#include "algorithm/Spiral.h"
#include "types/Position.h"

#include <cmath>

namespace Game3 {
	uint64_t getSpiralIndex(const Position &position) {
		// Credit: https://stackoverflow.com/a/30827007
		auto [y, x] = position;
		y = -y;

		const auto v = x - y;

		if (x + y > 0) {
			if (x >= y) {
				x <<= 1;
				return x * (x - 1) + v;
			}

			y <<= 1;
			return y * (y - 1) + v;
		}

		if (x < y) {
			x <<= 1;
			return x * (x - 1) - v;
		}

		y <<= 1;
		return y * (y - 1) - v;
	}

	Position getSpiralPosition(uint64_t index) {
		// Credit: https://gamedev.stackexchange.com/q/157291
		const double k = std::ceil((std::sqrt(index + 1) - 1) / 2);
		double t = 2 * k;
		double m = (t + 1) * (t + 1);

		if (index + 1 >= m - t)
			return {m - index - k - 1, -k};

		m -= t;

		if (index + 1 >= m - t)
			return {k, m - index - k - 1};

		m -= t;

		if (index + 1 >= m - t)
			return {k - m + index + 1, k};

		return {-k, k - m + index + 1 + t};
	}
}
