#pragma once

#include <cmath>

namespace Game3 {
	template <typename T>
	inline T updiv(T n, T d) {
		return n / d + (n % d? 1 : 0);
	}

	inline double fractional(double d) {
		return std::modf(d, &d);
	}
}
