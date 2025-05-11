#pragma once

#include <cstdint>

namespace Game3 {
	template <typename P, typename Fn>
	void iterateFilledCircle(P x0, P y0, P radius, const Fn &function) {
		// https://stackoverflow.com/a/14976268
		P x = radius;
		P y = 0;
		P x_change = 1 - (radius << 1);
		P y_change = 0;
		P radius_error = 0;

		while (x >= y) {
			for (P i = x0 - x; i <= x0 + x; ++i) {
				function(i, y0 + y);
				function(i, y0 - y);
			}

			for (P i = x0 - y; i <= x0 + y; ++i) {
				function(i, y0 + x);
				function(i, y0 - x);
			}

			++y;
			radius_error += y_change;
			y_change += 2;

			if ((radius_error << 1) + x_change > 0) {
				--x;
				radius_error += x_change;
				x_change += 2;
			}
		}
	}
}
