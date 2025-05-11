#pragma once

#include <cstdint>

namespace Game3 {
	template <typename P, typename Fn>
	void iterateFilledCircle(P x0, P y0, P radius, const Fn &function) {
		const P max = radius * radius + radius;
		for (P y = -radius; y <= radius; ++y) {
			for (P x = -radius; x <= radius; ++x) {
				if (x * x + y * y <= max) {
					function(x + x0, y + y0);
				}
			}
		}
	}
}
